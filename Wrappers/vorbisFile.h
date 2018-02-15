#pragma once

#define VISIT_PROCS(visit) \
	visit(ov_clear, jmpaddr) \
	visit(ov_open, jmpaddr) \
	visit(ov_open_callbacks, jmpaddr) \
	visit(ov_bitrate, jmpaddr) \
	visit(ov_bitrate_instant, jmpaddr) \
	visit(ov_streams, jmpaddr) \
	visit(ov_seekable, jmpaddr) \
	visit(ov_serialnumber, jmpaddr) \
	visit(ov_raw_total, jmpaddr) \
	visit(ov_pcm_total, jmpaddr) \
	visit(ov_time_total, jmpaddr) \
	visit(ov_raw_seek, jmpaddr) \
	visit(ov_pcm_seek, jmpaddr) \
	visit(ov_pcm_seek_page, jmpaddr) \
	visit(ov_time_seek, jmpaddr) \
	visit(ov_time_seek_page, jmpaddr) \
	visit(ov_raw_seek_lap, jmpaddr) \
	visit(ov_pcm_seek_lap, jmpaddr) \
	visit(ov_pcm_seek_page_lap, jmpaddr) \
	visit(ov_time_seek_lap, jmpaddr) \
	visit(ov_time_seek_page_lap, jmpaddr) \
	visit(ov_raw_tell, jmpaddr) \
	visit(ov_pcm_tell, jmpaddr) \
	visit(ov_time_tell, jmpaddr) \
	visit(ov_info, jmpaddr) \
	visit(ov_comment, jmpaddr) \
	visit(ov_read, jmpaddr) \
	visit(ov_read_float, jmpaddr) \
	visit(ov_test, jmpaddr) \
	visit(ov_test_callbacks, jmpaddr) \
	visit(ov_test_open, jmpaddr) \
	visit(ov_crosslap, jmpaddr) \
	visit(ov_halfrate, jmpaddr) \
	visit(ov_halfrate_p, jmpaddr) \
	visit(ov_fopen, jmpaddr)

PROC_CLASS(vorbisfile, dll)

#undef VISIT_PROCS
