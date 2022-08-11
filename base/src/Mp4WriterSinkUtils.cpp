#include <string>
#include <boost/filesystem.hpp>

#include "Logger.h"
#include "Mp4WriterSinkUtils.h"
#include <H264Utils.h>

Mp4WriterSinkUtils::Mp4WriterSinkUtils()
{
	lastVideoTS = 0;
	lastSyncTS = std::time(nullptr);
}

std::string Mp4WriterSinkUtils::format_hrs(int &hr)
{
	if (hr < 10)
	{
		return "000" + std::to_string(hr);
	}
	else
	{
		return "00" + std::to_string(hr);
	}
}

std::string Mp4WriterSinkUtils::format_2(int &num)
{
	if (num < 10)
	{
		return "0" + std::to_string(num);
	}
	else
	{
		return std::to_string(num);
	}
}

void Mp4WriterSinkUtils::parseTSJpeg(uint64_t &ts, uint32_t &chunkTimeInMinutes, uint32_t &syncTimeInMinutes,
	boost::filesystem::path &relPath, std::string &mp4FileName, bool &syncFlag)
{
	std::chrono::milliseconds duration(ts);
	std::chrono::seconds secondsInDuration = std::chrono::duration_cast<std::chrono::seconds>(duration);
	std::chrono::milliseconds msecsInDuration = std::chrono::duration_cast<std::chrono::milliseconds>(duration - secondsInDuration);

	std::chrono::time_point<std::chrono::system_clock> timePointInSeconds(secondsInDuration);
	std::time_t t = std::chrono::system_clock::to_time_t(timePointInSeconds);
	std::tm tm = *std::localtime(&t);
	uint16_t msecs = msecsInDuration.count();

	auto syncTimeInSecs = 60 * syncTimeInMinutes;
	if ((t - lastSyncTS) >= syncTimeInSecs)
	{
		syncFlag = true;
		lastSyncTS = t;
	}
	else
	{
		syncFlag = false;
	}

	// used cached values if the difference in ts is less than chunkTime
	uint32_t chunkTimeInSecs = 60 * chunkTimeInMinutes;
	if ((t - lastVideoTS) < chunkTimeInSecs)

	{
		relPath = lastVideoFolderPath;
		mp4FileName = lastVideoName;
		return;
	}

	// get new video path
	std::string yyyymmdd = std::to_string(1900 + tm.tm_year) + format_2(tm.tm_mon) + format_2(tm.tm_mday);
	relPath = boost::filesystem::path(yyyymmdd) / format_hrs(tm.tm_hour);
	mp4FileName = std::to_string(ts) + ".mp4";

	// cache new values
	lastVideoTS = t;
	lastVideoFolderPath = relPath;
	lastVideoMinute = tm.tm_min;
	lastVideoName = mp4FileName;
}

std::string Mp4WriterSinkUtils::getFilenameForNextFrameJpeg(uint64_t &timestamp, std::string &basefolder,
	uint32_t chunkTimeInMinutes, uint32_t syncTimeInMinutes, bool &syncFlag)
{
	boost::filesystem::path finalPath;
	std::string mp4FileName;
	boost::filesystem::path relPath;
	parseTSJpeg(timestamp, chunkTimeInMinutes, syncTimeInMinutes, relPath, mp4FileName, syncFlag);

	// create the folder if it does not exist
	auto folderPath = boost::filesystem::path(basefolder) / relPath;
	if (boost::filesystem::is_directory(folderPath))
	{
		finalPath = folderPath / mp4FileName;
		return finalPath.string();
	}

	if (boost::filesystem::create_directories(folderPath))
	{
		finalPath = folderPath / mp4FileName;
		return finalPath.string();
	}
	else
	{
		LOG_ERROR << "Failed to create the directory <" << folderPath << ">";
		LOG_ERROR << "Check the dir permissions.";
		return "";
	}
}

void Mp4WriterSinkUtils::parseTSH264(uint64_t& ts, uint32_t& chunkTimeInMinutes, uint32_t& syncTimeInMinutes,
	boost::filesystem::path& relPath, std::string& mp4FileName, bool& syncFlag, short& frameType)
{
	std::chrono::milliseconds duration(ts);
	std::chrono::seconds secondsInDuration = std::chrono::duration_cast<std::chrono::seconds>(duration);
	std::chrono::milliseconds msecsInDuration = std::chrono::duration_cast<std::chrono::milliseconds>(duration - secondsInDuration);

	std::chrono::time_point<std::chrono::system_clock> timePointInSeconds(secondsInDuration);
	std::time_t t = std::chrono::system_clock::to_time_t(timePointInSeconds);
	std::tm tm = *std::localtime(&t);
	uint16_t msecs = msecsInDuration.count();

	auto syncTimeInSecs = 60 * syncTimeInMinutes;
	if ((t - lastSyncTS) >= syncTimeInSecs)
	{
		syncFlag = true;
		lastSyncTS = t;
	}
	else
	{
		syncFlag = false;
	}
	// used cached values if the difference in ts is less than chunkTime
	uint32_t chunkTimeInSecs = 60 * chunkTimeInMinutes;

	if ((t - lastVideoTS) < chunkTimeInSecs)
	{
		relPath = lastVideoFolderPath;
		mp4FileName = lastVideoName;
		return;
	}
	
	if (frameType != H264Utils::H264_NAL_TYPE::H264_NAL_TYPE_IDR_SLICE)
	{
		relPath = lastVideoFolderPath;
		mp4FileName = lastVideoName;
		return;
	}
	// get new video path
	std::string yyyymmdd = std::to_string(1900 + tm.tm_year) + format_2(tm.tm_mon) + format_2(tm.tm_mday);
	relPath = boost::filesystem::path(yyyymmdd) / format_hrs(tm.tm_hour);
	mp4FileName = std::to_string(ts) + ".mp4";

	// cache new values
	lastVideoTS = t;
	lastVideoFolderPath = relPath;
	lastVideoMinute = tm.tm_min;
	lastVideoName = mp4FileName;
}

std::string Mp4WriterSinkUtils::getFilenameForNextFrameH264(uint64_t& timestamp, std::string& basefolder,
	uint32_t chunkTimeInMinutes, uint32_t syncTimeInMinutes, bool& syncFlag, short& frameType)
{
	boost::filesystem::path finalPath;
	std::string mp4FileName;
	boost::filesystem::path relPath;
	parseTSH264(timestamp, chunkTimeInMinutes, syncTimeInMinutes, relPath, mp4FileName, syncFlag, frameType);

	// create the folder if it does not exist
	auto folderPath = boost::filesystem::path(basefolder) / relPath;
	if (boost::filesystem::is_directory(folderPath))
	{
		finalPath = folderPath / mp4FileName;
		return finalPath.string();
	}

	if (boost::filesystem::create_directories(folderPath))
	{
		finalPath = folderPath / mp4FileName;
		return finalPath.string();
	}
	else
	{
		LOG_ERROR << "Failed to create the directory <" << folderPath << ">";
		LOG_ERROR << "Check the dir permissions.";
		return "";
	}
}

Mp4WriterSinkUtils::~Mp4WriterSinkUtils()
{
}
