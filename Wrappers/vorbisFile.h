#pragma once

#define VISIT_PROCS(visit) \
	visit(ov_clear) \
	visit(ov_open) \
	visit(ov_open_callbacks) \
	visit(ov_bitrate) \
	visit(ov_bitrate_instant) \
	visit(ov_streams) \
	visit(ov_seekable) \
	visit(ov_serialnumber) \
	visit(ov_raw_total) \
	visit(ov_pcm_total) \
	visit(ov_time_total) \
	visit(ov_raw_seek) \
	visit(ov_pcm_seek) \
	visit(ov_pcm_seek_page) \
	visit(ov_time_seek) \
	visit(ov_time_seek_page) \
	visit(ov_raw_seek_lap) \
	visit(ov_pcm_seek_lap) \
	visit(ov_pcm_seek_page_lap) \
	visit(ov_time_seek_lap) \
	visit(ov_time_seek_page_lap) \
	visit(ov_raw_tell) \
	visit(ov_pcm_tell) \
	visit(ov_time_tell) \
	visit(ov_info) \
	visit(ov_comment) \
	visit(ov_read) \
	visit(ov_read_float) \
	visit(ov_test) \
	visit(ov_test_callbacks) \
	visit(ov_test_open) \
	visit(ov_crosslap) \
	visit(ov_halfrate) \
	visit(ov_halfrate_p) \
	visit(ov_fopen)

PROC_CLASS(vorbisfile, dll)

#undef VISIT_PROCS
