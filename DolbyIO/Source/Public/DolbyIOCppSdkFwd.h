// Copyright 2023 Dolby Laboratories

#pragma once

namespace dolbyio::comms
{
	enum class conference_status;
	class refresh_token;
	class sdk;

	namespace plugin
	{
		class video_processor;
	}

	struct active_speaker_changed;
	struct audio_device_changed;
	struct audio_levels;
	struct conference_message_received;
	struct remote_participant_added;
	struct remote_participant_updated;
	struct remote_video_track_added;
	struct remote_video_track_removed;
	struct screen_share_error;

	namespace utils
	{
		struct vfs_event;
	}
}
