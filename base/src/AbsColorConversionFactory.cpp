#include "ColorConversion.h"
#include "AbsColorConversionFactory.h"


boost::shared_ptr<DetailAbs> AbsColorConversionFactory::create(std::string &version, uint16_t type)
{
	boost::shared_ptr<DetailAbs> mapper;
	static std::map<std::pair<std::string, int>, boost::shared_ptr<DetailAbs>> cache;
	auto requiredMapper = std::make_pair(version, type);

	if (cache.find(requiredMapper) != cache.end())
	{
		return cache[requiredMapper];
	}
	else
	{
		if (version == "v_1_0")
		{
			mapper = boost::shared_ptr<DetailAbs>(new EDAResultProtoMapper_v_1_0);
		}
		else if (version == "v_2_0")
		{
			if (type == AbsResultBase::RESULT_TYPE_CODE::DEFECTS_INFO)
			{
				mapper = boost::shared_ptr<DetailAbs>(new DefectsInfoMapper_v_2_0);
			}
			else if (type == AbsResultBase::RESULT_TYPE_CODE::SATURATION_RESULT)
			{
				mapper = boost::shared_ptr<DetailAbs>(new SaturationResultMapper_v_2_0);
			}
			else
			{
				LOG_ERROR << "No mapper found for: version <" << version << "> type <"<< std::to_string(type) << ">";
				throw AIPException(AIP_FATAL, "No mapper found for: version <" + version + "> type <" + std::to_string(type) + ">");
			}
		}
		else
		{
			LOG_ERROR << "No mapper found for: version <" << version + "> type <" << std::to_string(type) << ">";
			throw AIPException(AIP_FATAL, "No mapper found for: version <" + version + "> type <" + std::to_string(type) + ">");
		}
	}

	cache[requiredMapper] = mapper;
	return mapper;
}