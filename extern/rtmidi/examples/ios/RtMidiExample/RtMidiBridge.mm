#import "RtMidiBridge.h"
#import "RtMidi.h"
#include <memory>

@implementation RtMidiBridge {
    std::unique_ptr<RtMidiOut> _out;
    std::unique_ptr<RtMidiIn> _in;
    std::unique_ptr<RtMidiOut> _virtualOut;
    std::unique_ptr<RtMidiIn> _virtualIn;
    RtMidiReceiveBlock _receiveHandler;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        try {
            _out = std::make_unique<RtMidiOut>();
            _in = std::make_unique<RtMidiIn>();
        } catch (RtMidiError &e) {
            NSLog(@"RtMidiBridge init failed: %s", e.getMessage().c_str());
        }
        [self openVirtualPorts];
    }
    return self;
}

// A MIDI thru for other apps: "RtMidi In" goes straight out "RtMidi Out".
- (void)openVirtualPorts {
    try {
        _virtualOut = std::make_unique<RtMidiOut>();
        _virtualOut->openVirtualPort("RtMidi Out");
    } catch (RtMidiError &e) {
        NSLog(@"RtMidiBridge: couldn't open virtual port \"RtMidi Out\": %s", e.getMessage().c_str());
        _virtualOut.reset();
    }
    try {
        _virtualIn = std::make_unique<RtMidiIn>();
        _virtualIn->openVirtualPort("RtMidi In");
        _virtualIn->ignoreTypes(false, false, false);
        _virtualIn->setCallback([](double, std::vector<unsigned char> *message, void *userData) {
            RtMidiBridge *self = (__bridge RtMidiBridge *)userData;
            if (!self || !message || message->empty() || !self->_virtualOut) return;
            self->_virtualOut->sendMessage(message);
        }, (__bridge void *)self);
    } catch (RtMidiError &e) {
        NSLog(@"RtMidiBridge: couldn't open virtual port \"RtMidi In\": %s", e.getMessage().c_str());
        _virtualIn.reset();
    }
}

- (NSArray<NSString *> *)outputPortNames {
    NSMutableArray<NSString *> *names = [NSMutableArray array];
    if (!_out) return names;
    for (unsigned int i = 0; i < _out->getPortCount(); ++i) {
        [names addObject:[NSString stringWithUTF8String:_out->getPortName(i).c_str()]];
    }
    return names;
}

- (NSArray<NSString *> *)inputPortNames {
    NSMutableArray<NSString *> *names = [NSMutableArray array];
    if (!_in) return names;
    for (unsigned int i = 0; i < _in->getPortCount(); ++i) {
        [names addObject:[NSString stringWithUTF8String:_in->getPortName(i).c_str()]];
    }
    return names;
}

- (BOOL)openOutputPortAtIndex:(NSUInteger)index error:(NSError **)error {
    if (!_out) return NO;
    try {
        if (_out->isPortOpen()) _out->closePort();
        _out->openPort((unsigned int)index);
        return YES;
    } catch (RtMidiError &e) {
        if (error) {
            *error = [NSError errorWithDomain:@"RtMidiBridge" code:1
                userInfo:@{NSLocalizedDescriptionKey: [NSString stringWithUTF8String:e.getMessage().c_str()]}];
        }
        return NO;
    }
}

- (BOOL)openInputPortAtIndex:(NSUInteger)index error:(NSError **)error {
    if (!_in) return NO;
    try {
        if (_in->isPortOpen()) _in->closePort();
        _in->openPort((unsigned int)index);
        _in->ignoreTypes(false, false, false);
        _in->setCallback([](double deltatime, std::vector<unsigned char> *message, void *userData) {
            RtMidiBridge *self = (__bridge RtMidiBridge *)userData;
            if (!self || !message || message->empty()) return;
            NSData *data = [NSData dataWithBytes:message->data() length:message->size()];
            RtMidiReceiveBlock handler = self->_receiveHandler;
            if (handler) {
                dispatch_async(dispatch_get_main_queue(), ^{ handler(data, deltatime); });
            }
        }, (__bridge void *)self);
        return YES;
    } catch (RtMidiError &e) {
        if (error) {
            *error = [NSError errorWithDomain:@"RtMidiBridge" code:2
                userInfo:@{NSLocalizedDescriptionKey: [NSString stringWithUTF8String:e.getMessage().c_str()]}];
        }
        return NO;
    }
}

- (void)closeOutputPort {
    if (_out && _out->isPortOpen()) _out->closePort();
}

- (void)closeInputPort {
    if (_in && _in->isPortOpen()) _in->closePort();
}

- (void)setReceiveHandler:(RtMidiReceiveBlock)handler {
    _receiveHandler = [handler copy];
}

- (void)sendBytes:(NSData *)bytes {
    if (!_out || !_out->isPortOpen()) return;
    std::vector<unsigned char> msg((const unsigned char *)bytes.bytes,
                                    (const unsigned char *)bytes.bytes + bytes.length);
    _out->sendMessage(&msg);
}

@end
