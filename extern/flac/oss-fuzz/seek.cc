/* fuzzer_seek
 * Copyright (C) 2022-2025  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <cstring> /* for memcpy */
#include "FLAC/stream_decoder.h"
#include "common.h"

#if MSAN == 1
extern "C" void __msan_check_mem_is_initialized(const volatile void *x, size_t size);
#endif

int write_abort_check_counter = -1;
int written_uncompressed_bytes = 0;
int errors_received_counter = 0;

#if 0 /* set to 1 to debug */
#define FPRINTF_DEBUG_ONLY(...) fprintf(__VA_ARGS__)
#else
#define FPRINTF_DEBUG_ONLY(...)
#endif

#define CONFIG_LENGTH 3

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	(void)decoder, (void)buffer, (void)client_data;
	if(write_abort_check_counter > 0) {
		write_abort_check_counter--;
		if(write_abort_check_counter == 0)
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	} else if(write_abort_check_counter == 0)
		/* This must not happen: write callback called after abort is returned */
		abort();

	written_uncompressed_bytes += frame->header.blocksize * frame->header.channels * frame->header.bits_per_sample / 8;
	if(written_uncompressed_bytes > (1 << 24))
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;


	if(errors_received_counter > 10000)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus error, void *client_data)
{
	(void)decoder, (void)error, (void)client_data;
	errors_received_counter++;
}


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	FLAC__bool decoder_valid = true;
	FLAC__StreamDecoder *decoder;
	uint8_t command_length;
	FLAC__bool init_bools[16], ogg;

	if(size < 1)
		return 1;

	if(data[0] < 128) /* Use MSB as on/off */
		alloc_check_threshold = data[0];
	else
		alloc_check_threshold = INT32_MAX;
	alloc_check_counter = 0;

	write_abort_check_counter = -1;
	written_uncompressed_bytes = 0;
	errors_received_counter = 0;

	/* allocate the decoder */
	if((decoder = FLAC__stream_decoder_new()) == NULL) {
		return 1;
	}

	/* Use first CONFIG_LENGTH bytes for configuration, leave at least one byte of input */
	if(size < 1 + CONFIG_LENGTH){
		FLAC__stream_decoder_delete(decoder);
		return 0;
	}

	/* bit 8 to 19 bits for configuration bools, bit 20 to 23 for length of command section */
	for(int i = 0; i < 12; i++)
		init_bools[i] = data[1+i/8] & (1 << (i % 8));

	command_length = data[CONFIG_LENGTH-1] >> 4;

	/* Leave at least one byte as input */
	if(command_length >= size - 1 - CONFIG_LENGTH)
		command_length = size - 1 - CONFIG_LENGTH;

	/* Dump decoder input to file */
	{
		FILE * file_to_decode = fopen("/tmp/tmp.flac","w");
		fwrite(data+CONFIG_LENGTH+command_length,1,size-CONFIG_LENGTH-command_length,file_to_decode);
		fclose(file_to_decode);
	}

	ogg = init_bools[0];

	FLAC__stream_decoder_set_md5_checking(decoder,init_bools[1]);
	if(init_bools[2])
		FLAC__stream_decoder_set_metadata_respond_all(decoder);
	if(init_bools[3])
		FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	if(init_bools[4])
		FLAC__stream_decoder_set_decode_chained_stream(decoder, true);

	/* initialize decoder */
	if(decoder_valid) {
		FLAC__StreamDecoderInitStatus init_status;
		if(ogg)
			init_status = FLAC__stream_decoder_init_ogg_file(decoder, "/tmp/tmp.flac", write_callback, NULL, error_callback, NULL);
		else
			init_status = FLAC__stream_decoder_init_file(decoder, "/tmp/tmp.flac", write_callback, NULL, error_callback, NULL);
		if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
			decoder_valid = false;
		}
	}

	/* Run commands */
	for(uint8_t i = 0; decoder_valid && (i < command_length); i++){
		const uint8_t * command = data+CONFIG_LENGTH+i;
		uint8_t shift = 1u << (command[0] >> 3);
		FLAC__uint64 seekpos;

		switch(command[0] & 15){
			case 0:
				FPRINTF_DEBUG_ONLY(stderr,"end_of_stream\n");
				decoder_valid = FLAC__stream_decoder_process_until_end_of_stream(decoder);
				break;
			case 1:
				FPRINTF_DEBUG_ONLY(stderr,"end_of_metadata\n");
				decoder_valid = FLAC__stream_decoder_process_until_end_of_metadata(decoder);
				break;
			case 2:
				FPRINTF_DEBUG_ONLY(stderr,"single\n");
				decoder_valid = FLAC__stream_decoder_process_single(decoder);
				break;
			case 3:
				FPRINTF_DEBUG_ONLY(stderr,"skip_single\n");
				decoder_valid = FLAC__stream_decoder_skip_single_frame(decoder);
				break;
			case 4:
				FPRINTF_DEBUG_ONLY(stderr,"reset\n");
				decoder_valid = FLAC__stream_decoder_reset(decoder);
				break;
			case 5:
				FPRINTF_DEBUG_ONLY(stderr,"flush\n");
				decoder_valid = FLAC__stream_decoder_flush(decoder);
				break;
			case 6:
			case 14:
				shift = 1u << (command[0] >> 3);
				FPRINTF_DEBUG_ONLY(stderr,"seek short %hhu\n",shift);
				decoder_valid = FLAC__stream_decoder_seek_absolute(decoder,shift);
				break;
			case 7:
				if(i+8 >= command_length) /* Not enough data available to do this */
					break;
				seekpos = ((FLAC__uint64)command[1] << 56) +
				          ((FLAC__uint64)command[2] << 48) +
				          ((FLAC__uint64)command[3] << 40) +
				          ((FLAC__uint64)command[4] << 32) +
				          ((FLAC__uint64)command[5] << 24) +
				          ((FLAC__uint64)command[6] << 16) +
				          ((FLAC__uint64)command[7] << 8) +
				          command[8];
				i+=8;
				FPRINTF_DEBUG_ONLY(stderr,"seek long %lu\n",seekpos);
				decoder_valid = FLAC__stream_decoder_seek_absolute(decoder,seekpos);
				break;
			case 8:
				/* Set abort on write callback */
				write_abort_check_counter = (command[0] >> 4) + 1;
				break;
			case 9:
				FPRINTF_DEBUG_ONLY(stderr,"end_of_link\n");
				decoder_valid = FLAC__stream_decoder_process_until_end_of_link(decoder);
				break;
			case 10:
				FPRINTF_DEBUG_ONLY(stderr,"finish_link\n");
				if(FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_END_OF_LINK)
					FLAC__stream_decoder_finish_link(decoder);
				break;
			case 11:
				FPRINTF_DEBUG_ONLY(stderr,"skip_single_link\n");
				decoder_valid = FLAC__stream_decoder_skip_single_link(decoder);
				break;
			case 12:
				FPRINTF_DEBUG_ONLY(stderr,"find_total_samples\n");
				if(FLAC__stream_decoder_find_total_samples(decoder) == 0) {
					FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(decoder);
					if(state == FLAC__STREAM_DECODER_OGG_ERROR ||
					state == FLAC__STREAM_DECODER_SEEK_ERROR ||
					state == FLAC__STREAM_DECODER_ABORTED ||
					state == FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR ||
					state == FLAC__STREAM_DECODER_UNINITIALIZED)
						decoder_valid = false;
				}
				break;
			case 13:
				int32_t retval;
				FLAC__uint64 *link_lengths;
				FPRINTF_DEBUG_ONLY(stderr,"get_link_lengths\n");
				retval = FLAC__stream_decoder_get_link_lengths(decoder, &link_lengths);
				if(retval == FLAC__STREAM_DECODER_GET_LINK_LENGTHS_MEMORY_ALLOCATION_ERROR) {
					decoder_valid = false;
				}
				if(retval > 0) {
					for(int32_t j = 0; j < retval; j++) {
#if MSAN == 1
						 __msan_check_mem_is_initialized(&link_lengths[j],sizeof(link_lengths[j]));
#else
						;
#endif
					}
					free(link_lengths);
				}
				break;
			/* case 14 is already used above */
		}
		if(!decoder_valid) {
			/* Try again if possible */
			FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(decoder);
			if(state != FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR && state != FLAC__STREAM_DECODER_ABORTED) {
				FPRINTF_DEBUG_ONLY(stderr,"reset invalid\n");
				decoder_valid = FLAC__stream_decoder_reset(decoder);
			}
		}
	}

	FLAC__stream_decoder_finish(decoder);

	FLAC__stream_decoder_delete(decoder);

	return 0;
}

