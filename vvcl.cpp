#include "vvcl.h"

#if defined(_MSC_VER)
#define INT64_C(val) val##i64
#define UINT64_C(val) val##ui64
#elif defined(__GNUC__)
#define INT64_C(val) val##LL
#define UINT64_C(val) val##ULL
#endif

extern "C"{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

typedef struct{
	const char Name[MAX_NAME];
	const char* FormatName;
	AVCodecID VideoEncoder;
	float Scale;
}RECORDER_CONFIG;

static const RECORDER_CONFIG s_Config[] = {
	{ "AVI/H.263+ / H.263-1998 / H.263 version 2", "avi", AV_CODEC_ID_H263P, 0.05f },
	{ "AVI/MPEG-4 part 2", "avi", AV_CODEC_ID_MPEG4, 0.05f },
	{ "AVI/MPEG-4 part 2 Microsoft variant version 3", "avi", AV_CODEC_ID_MSMPEG4V3, 0.05f },
	{ "AVI/MPEG-4 part 2 Microsoft variant version 2", "avi", AV_CODEC_ID_MSMPEG4V2, 0.05f },
	{ "AVI/Windows Media Video 7", "avi", AV_CODEC_ID_WMV1, 0.05f },
	{ "AVI/Windows Media Video 8", "avi", AV_CODEC_ID_WMV2, 0.05f },
	{ "AVI/MPEG-1 video", "avi", AV_CODEC_ID_MPEG1VIDEO, 0.05f },
	{ "AVI/MPEG-2 video", "avi", AV_CODEC_ID_MPEG2VIDEO, 0.05f },
	{ "AVI/MJPEG (Motion JPEG)", "avi", AV_CODEC_ID_MJPEG, 0.05f },
	{ "AVI/Lossless JPEG", "avi", AV_CODEC_ID_LJPEG, 0.05f },
	{ "AVI/JPEG-LS", "avi", AV_CODEC_ID_JPEGLS, 0.05f },
	{ "AVI/Huffyuv / HuffYUV", "avi", AV_CODEC_ID_HUFFYUV, 0.05f },
	{ "AVI/Huffyuv FFmpeg variant", "avi", AV_CODEC_ID_FFVHUFF, 0.05f },
	{ "AVI/ASUS V1", "avi", AV_CODEC_ID_ASV1, 0.05f },
	{ "AVI/ASUS V2", "avi", AV_CODEC_ID_ASV2, 0.05f },
	{ "AVI/FFmpeg video codec #1", "avi", AV_CODEC_ID_FFV1, 0.05f },
	{ "AVI/Flash Video (FLV) / Sorenson Spark / Sorenson H.263", "avi", AV_CODEC_ID_FLV1, 0.05f },
	{ "AVI/Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1", "avi", AV_CODEC_ID_SVQ1, 0.05f },
	{ "AVI/DPX image", "avi", AV_CODEC_ID_DPX, 0.05f },
	{ "AVI/AMV Video", "avi", AV_CODEC_ID_AMV, 0.05f },
	{ "MP4/MPEG-4 part 2", "mp4", AV_CODEC_ID_MPEG4, 0.05f },
};

static const int s_ConfigCount = sizeof(s_Config) / sizeof(s_Config[0]);

static bool s_ffmpeg_initialized = false;

static const char s_ffmpeg_log_prefix[] = "FFMPEG: ";

typedef struct{
	int width;
	int height;
	int offset;
	AVPicture PictureIn;
	AVFormatContext* pFile;
	bool bNeedCloseCodec;
	AVFrame* pFrame;
	SwsContext* Resample;
	int MaxPacketSize;
	AVPacket Packet;
}RECORDER;

static void ffmpeg_log(void* , int level, const char* fmt, va_list args)
{
	if(level > AV_LOG_VERBOSE){
		return;
	}

	char s[64 * 1024];

	memcpy(s, s_ffmpeg_log_prefix, sizeof(s_ffmpeg_log_prefix));

	vsnprintf(s + sizeof(s_ffmpeg_log_prefix) - 1, sizeof(s) - sizeof(s_ffmpeg_log_prefix) - 1, fmt, args);

	printf(s);
}

static void ffmpeg_init(void)
{
	if(!s_ffmpeg_initialized){
		av_log_set_callback(ffmpeg_log);
		avcodec_register_all();
		av_register_all();
		s_ffmpeg_initialized = true;
	}
}

char getAvailableEncoderName(int encoderIndex, char encoderName[MAX_NAME])
{
	if(encoderName == NULL){
		return RECORDER_ERROR;
	}
	
	if(encoderIndex < 0 || encoderIndex >= s_ConfigCount){
		encoderName[0] = '\0';
		return RECORDER_ERROR;
	}

	strcpy(encoderName, s_Config[encoderIndex].Name);

	return RECORDER_OK;
}

static RECORDER Recorder;

char recorderInitialize(int resolutionX, int resolutionY, const char* fileAndPath, int framerate, int encoderIndex)
{
	if(encoderIndex < 0 || encoderIndex >= s_ConfigCount){
		return RECORDER_ERROR;
	}

	if(framerate < 1){
		return RECORDER_ERROR;
	}

	if(fileAndPath == NULL){
		return RECORDER_ERROR;
	}

	if(resolutionX < 64 - 15 || resolutionY < 64 - 15){
		return RECORDER_ERROR;
	}

	ffmpeg_init();

	RECORDER* r = &Recorder;

	do{
		memset(r, 0, sizeof(*r));

		r->width = resolutionX;
		r->height = resolutionY;

		const int width = ((resolutionX + 15) / 16) * 16;
		const int height = ((resolutionY + 15) / 16) * 16;


		if(avpicture_alloc(&r->PictureIn, AV_PIX_FMT_RGB24, width, height) < 0){
			break;
		}
		
		r->offset = (height - r->height) / 2 * r->PictureIn.linesize[0] + (width - r->width) / 2 * 3;

		int y;

		for(y = 0; y < height; y++){
			unsigned char* p = r->PictureIn.data[0] + r->PictureIn.linesize[0] * y;

			memset(p, 0, r->PictureIn.linesize[0]);
		}

		const RECORDER_CONFIG* e = &s_Config[encoderIndex];

		if(avformat_alloc_output_context2(&r->pFile, NULL, e->FormatName, fileAndPath) < 0){
			break;
		}

		AVStream* s;

		if((s = avformat_new_stream(r->pFile, NULL)) == NULL){
			break;
		}

		AVCodecContext* c = s->codec;

		AVCodec* pCodec = avcodec_find_encoder(e->VideoEncoder);

		if(pCodec == NULL){
			break;
		}
    
		avcodec_get_context_defaults3(c, pCodec);

		c->codec_id = pCodec->id;

		c->width = width;
		c->height = height;
		c->bit_rate = (const int )((c->width * c->height *  3 / 2 * framerate * 8) * e->Scale);
		c->time_base.den = framerate;
		c->time_base.num = 1;
		c->gop_size = framerate * 5;
		c->pix_fmt = AV_PIX_FMT_YUV420P;
		c->max_b_frames = 0;

		if(c->codec_id == AV_CODEC_ID_MJPEG || c->codec_id == AV_CODEC_ID_LJPEG || c->codec_id == AV_CODEC_ID_AMV){
			c->pix_fmt = AV_PIX_FMT_YUVJ420P;
		}

		if(c->codec_id == AV_CODEC_ID_JPEGLS || c->codec_id == AV_CODEC_ID_HUFFYUV || c->codec_id == AV_CODEC_ID_DPX){
			c->pix_fmt = AV_PIX_FMT_RGB24;
		}

		if(c->codec_id == AV_CODEC_ID_SVQ1){
			c->pix_fmt = AV_PIX_FMT_YUV410P;
		}

		if(c->codec_id == AV_CODEC_ID_TARGA){
			c->pix_fmt = AV_PIX_FMT_BGR24;
		}

		if(c->codec_id == AV_CODEC_ID_MPEG1VIDEO){
			c->mb_decision = 2;
		}

		if(r->pFile->oformat->flags & AVFMT_GLOBALHEADER){
			c->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}

		if(avcodec_open2(c, pCodec, NULL) < 0){
			break;
		}

		r->bNeedCloseCodec = true;

		if((r->pFrame = av_frame_alloc()) == NULL){
			break;
		}

		if(avpicture_alloc((AVPicture* )r->pFrame, c->pix_fmt, c->width, c->height) < 0){
			break;
		}

		r->pFrame->pts = 0;

		if((r->Resample = sws_getContext(c->width, c->height, AV_PIX_FMT_RGB24, c->width, c->height, c->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL)) == NULL){
			break;
		}

		if(avio_open(&r->pFile->pb, r->pFile->filename, 2) < 0){
			break;
		}

		if(avformat_write_header(r->pFile, NULL) < 0){
			break;
		}

		r->MaxPacketSize = c->width * c->height * 8;

		if(av_new_packet(&r->Packet, r->MaxPacketSize) < 0){
			break;
		}

		r->Packet.stream_index = s->index;

		if((r->width & 15) != 0 || (r->height & 15) != 0){
			return RECORDER_WARNING;
		}

		return RECORDER_OK;
	}while(0);

	recorderEnd();

	return RECORDER_ERROR;
}

char recorderAddFrame(const unsigned char* buffer)
{
	RECORDER* r = &Recorder;

	if(r->pFile == NULL || r->pFile->nb_streams != 1){
		return RECORDER_ERROR;
	}

	AVStream* s = r->pFile->streams[0];

	if(s == NULL){
		return RECORDER_ERROR;
	}

	AVCodecContext* c = s->codec;

	if(c == NULL){
		return RECORDER_ERROR;
	}

	AVFrame* pFrame = NULL;

	if(buffer != NULL){
		AVPicture Src;

		memset(&Src, 0, sizeof(Src));

		Src.data[0] = (uint8_t* )buffer;
		Src.linesize[0] = r->width * 3;

		const bool bSrcFlip = false;

		if(bSrcFlip != false){
			Src.data[0] += Src.linesize[0] * (r->height - 1);
			Src.linesize[0] = -Src.linesize[0];
		}

		int y;
		
		for(y = 0; y < r->height; y++){
			const unsigned char* s = Src.data[0] + Src.linesize[0] * y;
			unsigned char* d = r->PictureIn.data[0] + r->offset + r->PictureIn.linesize[0] * y;

			memcpy(d, s, r->width * 3);
		}

		if(sws_scale(r->Resample, r->PictureIn.data, r->PictureIn.linesize, 0, c->height, r->pFrame->data, r->pFrame->linesize) < 0){
			return RECORDER_ERROR;
		}
		
		pFrame = r->pFrame;
	}

	//if((r->Packet.size = avcodec_encode_video(c, r->Packet.data, r->MaxPacketSize, pFrame)) < 0){
    int got_packet;
    if(avcodec_encode_video2(c, &r->Packet, pFrame, &got_packet) < 0){
		return RECORDER_ERROR;
	}

	if(pFrame != NULL){
		pFrame->pts += 1;

		//if(r->Packet.size == 0){
		if(!got_packet){
			return RECORDER_OK;
		}
	}else{
		//if(r->Packet.size == 0){
		if(!got_packet){
			return RECORDER_ERROR;
		}
	}

	r->Packet.pts = r->Packet.dts = AV_NOPTS_VALUE;

	if(c->coded_frame->pts != AV_NOPTS_VALUE){
		r->Packet.pts = av_rescale_q(c->coded_frame->pts, c->time_base, s->time_base);
	}

	r->Packet.flags &= ~AV_PKT_FLAG_KEY;

	if(c->coded_frame->key_frame != 0){
		r->Packet.flags |= AV_PKT_FLAG_KEY;
	}
	
	if(av_write_frame(r->pFile, &r->Packet) < 0){
		return RECORDER_ERROR;
	}

	return RECORDER_OK;
}

char recorderEnd(void)
{
	RECORDER* r = &Recorder;

	av_free_packet(&r->Packet);

	if(r->Resample != NULL){
		sws_freeContext(r->Resample);
		r->Resample = NULL;
	}

	if(r->pFrame != NULL){
		if(r->pFrame->data[0] != NULL){
			avpicture_free((AVPicture* )r->pFrame);
		}

		av_freep(&r->pFrame);
	}

	if(r->pFile != NULL){
		if(r->pFile->pb != NULL){
			av_write_trailer(r->pFile);
		}

		unsigned i;

		for(i = 0; i < r->pFile->nb_streams; i++){
			if(r->pFile->streams[i] != NULL){
				if(r->pFile->streams[i]->codec != NULL){
					AVCodecContext* c = r->pFile->streams[i]->codec;

					if(r->bNeedCloseCodec){
						avcodec_close(c);
						r->bNeedCloseCodec = false;
					}

					if(c->extradata_size > 0 && c->extradata != NULL){
						av_freep(&c->extradata);
					}

					c->extradata_size = 0;

 					av_freep(&c->codec);
				}

				av_freep(&r->pFile->streams[i]);
			}
		}

		if(r->pFile->pb != NULL){
			avio_close(r->pFile->pb);
		}

		av_freep(&r->pFile);
	}

	if(r->PictureIn.data[0] != NULL){
		avpicture_free(&r->PictureIn);
	}

	return RECORDER_OK;
}
