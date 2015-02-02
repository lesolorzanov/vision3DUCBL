#ifndef OpenNI_Wrapper_OpenCV
#define OpenNI_Wrapper_OpenCV

namespace OpenNI2WrapperOpenCV {

const char* PixelFormatToStr(openni::PixelFormat pixelformat) {
    switch (pixelformat) {
        case openni::PIXEL_FORMAT_DEPTH_100_UM:
            return "PIXEL_FORMAT_DEPTH_100_UM";
            break;
        case openni::PIXEL_FORMAT_DEPTH_1_MM:
            return "PIXEL_FORMAT_DEPTH_1_MM";
            break;
        case openni::PIXEL_FORMAT_GRAY16:
            return "PIXEL_FORMAT_GRAY16";
            break;
        case openni::PIXEL_FORMAT_GRAY8:
            return "PIXEL_FORMAT_GRAY8";
            break;
        case openni::PIXEL_FORMAT_JPEG:
            return "PIXEL_FORMAT_JPEG";
            break;
        case openni::PIXEL_FORMAT_RGB888:
            return "PIXEL_FORMAT_RGB888";
            break;
        case openni::PIXEL_FORMAT_SHIFT_9_2:
            return "PIXEL_FORMAT_SHIFT_9_2";
            break;
        case openni::PIXEL_FORMAT_SHIFT_9_3:
            return "PIXEL_FORMAT_SHIFT_9_3";
            break;
        case openni::PIXEL_FORMAT_YUV422:
            return "PIXEL_FORMAT_YUV422";
            break;
        case openni::PIXEL_FORMAT_YUYV:
            return "PIXEL_FORMAT_YUYV";
            break;
        default:
            return "unknown";
            break;
    }
}

int InitializeDevice ( char* deviceURI, openni::Device& m_device ) {
	openni::Status rc = openni::STATUS_OK;
	rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK)
	{
		printf("OpenNI2WrapperOpenCV: OpenNI initialization failed:\n%s\n", openni::OpenNI::getExtendedError());
		return rc;
	}
	rc = m_device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		printf("OpenNI2WrapperOpenCV: Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return rc;
	}
	return openni::STATUS_OK;
}

int OpenStream ( const openni::Device& m_device, openni::SensorType streamType, openni::VideoStream& m_stream ) {
	openni::Status rc = openni::STATUS_OK;
	rc = m_stream.create(m_device, streamType);
	if (rc != openni::STATUS_OK)
	{
		printf("OpenNI2WrapperOpenCV: Couldn't find stream:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return rc;
	}
	if (!m_stream.isValid())
	{
		printf("OpenNI2WrapperOpenCV: Stream is not valid.\n");
		m_stream.destroy();
		openni::OpenNI::shutdown();
		return rc;
	}
	return openni::STATUS_OK;
}

/*
void tmp () {
	XnBool isSupported = depthGenerator.IsCapabilitySupported("AlternativeViewPoint");
	if(TRUE == isSupported)
	{
		XnStatus res = depthGenerator.GetAlternativeViewPointCap().SetViewPoint(imageGenerator);
		if(XN_STATUS_OK != res)
		{
			printf("Getting and setting AlternativeViewPoint failed: %s\n", xnGetStatusString(res));
		}
	}
}

void changeRegistration(int nValue)
{
	if (!g_Depth.IsValid() || !g_Depth.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT))
	{
		return;
	}

	if (!nValue)
	{
		g_Depth.GetAlternativeViewPointCap().ResetViewPoint();
	}
	else if (g_Image.IsValid())
	{
		g_Depth.GetAlternativeViewPointCap().SetViewPoint(g_Image);
	}
}
*/

int EnumerateStreamVideoModes ( const openni::VideoStream& m_stream ) {
	const openni::SensorInfo& si = m_stream.getSensorInfo();
	std::cout << "stream supported res " << endl;
	for (int i=0; i<si.getSupportedVideoModes().getSize(); i++) {
		cout << i << ": " << si.getSupportedVideoModes()[i].getResolutionX() << "x" << si.getSupportedVideoModes()[i].getResolutionY() << ": " << PixelFormatToStr(si.getSupportedVideoModes()[i].getPixelFormat()) << endl;
	}
	return openni::STATUS_OK;
}

int StartStreamWithVideoMode ( openni::VideoStream& m_stream, int videoMode ) {
	openni::Status rc = openni::STATUS_OK;
	const openni::SensorInfo& si = m_stream.getSensorInfo();
	rc = m_stream.setVideoMode(si.getSupportedVideoModes()[videoMode]);
	if (rc != openni::STATUS_OK)
	{
		printf("OpenNI2WrapperOpenCV: Unsupported video mode:\n%s\n", openni::OpenNI::getExtendedError());
		m_stream.destroy();
		openni::OpenNI::shutdown();
		return rc;
	}
	rc = m_stream.start();
	if (rc != openni::STATUS_OK)
	{
		printf("OpenNI2WrapperOpenCV: Couldn't start stream:\n%s\n", openni::OpenNI::getExtendedError());
		m_stream.destroy();
		openni::OpenNI::shutdown();
		return rc;
	}
	return openni::STATUS_OK;
}

void SetFrameRegistrationMode ( openni::Device& m_device, bool reg ) {
	openni::ImageRegistrationMode mode = reg ? openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR : openni::IMAGE_REGISTRATION_OFF;
	if ( m_device.isImageRegistrationModeSupported ( mode ) ) {
		printf("OpenNI2WrapperOpenCV: Device set registration mode : %d\n", mode );
		m_device.setImageRegistrationMode ( mode );
	} else {
		printf("OpenNI2WrapperOpenCV: Device can't set registration mode\n");
	}
}

int GrabFrameFromStream ( openni::VideoStream& m_stream, openni::VideoFrameRef& m_frame ) {
	m_stream.readFrame(&m_frame);
	if (m_frame.isValid()) {
		return openni::STATUS_OK;
	}
	return openni::STATUS_ERROR;
}

int ConvertDepthFrameToStream ( const openni::VideoFrameRef& m_depthFrame, cv::Mat& frame ) {
	if (m_depthFrame.isValid()) {
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)m_depthFrame.getData();
                memcpy(frame.data,pDepthRow,m_depthFrame.getStrideInBytes() * m_depthFrame.getHeight());
		return openni::STATUS_OK;
	}
	return openni::STATUS_ERROR;
}

int ConvertColorFrameToStream (  const openni::VideoFrameRef& m_colorFrame, cv::Mat& frame ) {
	if (m_colorFrame.isValid()) {
                const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
                memcpy(frame.data,pImageRow,m_colorFrame.getStrideInBytes() * m_colorFrame.getHeight());
                cvtColor(frame, frame, COLOR_RGB2BGR);
		return openni::STATUS_OK;
	}
	return openni::STATUS_ERROR;
}

}

#endif
