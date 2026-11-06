#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^RtMidiReceiveBlock)(NSData *bytes, double deltaTime);

@interface RtMidiBridge : NSObject

// Opens "RtMidi In" and "RtMidi Out" at launch as a thru for other apps.
- (NSArray<NSString *> *)outputPortNames;
- (NSArray<NSString *> *)inputPortNames;

- (BOOL)openOutputPortAtIndex:(NSUInteger)index error:(NSError **)error NS_SWIFT_NAME(openOutputPort(at:));
- (BOOL)openInputPortAtIndex:(NSUInteger)index error:(NSError **)error NS_SWIFT_NAME(openInputPort(at:));

- (void)closeOutputPort;
- (void)closeInputPort;

- (void)setReceiveHandler:(nullable RtMidiReceiveBlock)handler;
- (void)sendBytes:(NSData *)bytes;

@end

NS_ASSUME_NONNULL_END
