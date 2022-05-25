#pragma once

#include "FrameMetadata.h"

class PropsChangeMetadata : public FrameMetadata
{
public:
	enum ModuleName
	{
		Module = 0,
		CalcHistogramCV,
		ChangeDetection,
		EdgeDefectAnalyis,
		FrameReaderModule,
		EffectsNPPI,
		OverlayNPPI,
		GPIOSink,
		GaussianBlur,
		NonmaxSuppression,
		HysteresisThreshold,
		HoughLinesCV,
		FindEdge,
		FindDefects,
		ConnectedComponents,
		Mp4WriterSink,
		ProtoSerializer,
		ProtoDeserializer,
		SaturationDetection,
		Mp4ReaderSource
	};

	PropsChangeMetadata(ModuleName _moduleName) : FrameMetadata(FrameType::PROPS_CHANGE), moduleName(_moduleName) {}

	ModuleName getModuleName() { return moduleName; }

private:
	ModuleName moduleName;
};
