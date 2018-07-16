//------------------------------------------------------------------------------
// <copyright file="DepthBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <strsafe.h>
#include "KinectSensor.h"
#include "resource.h"

#include <iostream>
#include <windows.h>
/// <summary>
/// Constructor
/// </summary>
KinectSensor::KinectSensor() :
    m_hNextDepthFrameEvent(INVALID_HANDLE_VALUE),
	m_hNextColorFrameEvent(INVALID_HANDLE_VALUE),
    m_pDepthStreamHandle(INVALID_HANDLE_VALUE),
	m_pColorStreamHandle(INVALID_HANDLE_VALUE),
	m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
	m_pSkeletonStreamHandle(INVALID_HANDLE_VALUE),
    m_bNearMode(false),
    m_pNuiSensor(NULL)
{
    // create heap storage for depth pixel data in RGBX format
    m_depthRGBX = new BYTE[cDepthWidth*cDepthHeight*cBytesPerPixel];
	depthValues = new USHORT[cDepthWidth*cDepthHeight];
	colorsRGBValues = new unsigned short[cDepthWidth * cDepthHeight * 3];
	m_depthD16 = new USHORT[cDepthWidth * cDepthHeight * 2]; 
	m_colorCoordinates = new LONG[cDepthWidth*cDepthHeight * 2];

	m_colorToDepthDivisor = 1;
}

/// <summary>
/// Destructor
/// </summary>
KinectSensor::~KinectSensor()
{
    if (m_pNuiSensor)
    {
        m_pNuiSensor->NuiShutdown();
    }

    if (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hNextDepthFrameEvent);
    }

    // done with depth pixel data
    delete[] m_depthRGBX;
	delete[] colorsRGBValues;
	delete[] depthValues;
	delete[] m_colorCoordinates;
	delete[] m_depthD16;

    SafeRelease(m_pNuiSensor);
	if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextSkeletonEvent);
	}
	if (m_hNextDepthFrameEvent && (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextDepthFrameEvent);
	}
	if (m_hNextColorFrameEvent && (m_hNextColorFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextColorFrameEvent);
	}
}

/// <summary>
/// Main processing function
/// </summary>
void KinectSensor::Update()
{
    if (NULL == m_pNuiSensor)
    {
        return;
    }

    if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0) )
    {
        ProcessDepth();
	}
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextColorFrameEvent, 0))
	{
		ProcessColor();
	}
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0))
	{
		ProcessSkeleton();
	}
}

/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT KinectSensor::CreateFirstConnected()
{
    INuiSensor * pNuiSensor;
    HRESULT hr;

    int iSensorCount = 0;
    hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
    {
        return hr;
    }

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            m_pNuiSensor = pNuiSensor;
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        pNuiSensor->Release();
    }

    if (NULL != m_pNuiSensor)
    {
        // Initialize the Kinect and specify that we'll be using depth
        hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH);
        if (SUCCEEDED(hr))
        {
            // Create an event that will be signaled when depth data is available
            m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            // Open a depth image stream to receive depth frames
            hr = m_pNuiSensor->NuiImageStreamOpen(
                NUI_IMAGE_TYPE_DEPTH,
                NUI_IMAGE_RESOLUTION_640x480,
                0,
                2,
                m_hNextDepthFrameEvent,
                &m_pDepthStreamHandle);

			// Create an event that will be signaled when color data is available
			m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			// Open a color image stream to receive color frames
			hr = m_pNuiSensor->NuiImageStreamOpen(
				NUI_IMAGE_TYPE_COLOR,
				NUI_IMAGE_RESOLUTION_640x480,
				0,
				2,
				m_hNextColorFrameEvent,
				&m_pColorStreamHandle);

			// Create an event that will be signaled when skeleton data is available
			m_hNextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

			// Open a skeleton stream to receive skeleton data
			hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);
        }
    }

    if (NULL == m_pNuiSensor || FAILED(hr))
    {
        SetStatusMessage(L"No ready Kinect found!");
        return E_FAIL;
    }

    return hr;
}

void KinectSensor::DisConnected() {
	if (m_pNuiSensor)
		m_pNuiSensor->NuiShutdown();
}

/// <summary>
/// Handle new depth data
/// </summary>
void KinectSensor::ProcessDepth()
{
    HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;
    // Attempt to get the depth frame
    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
    if (FAILED(hr))	
    {
        return;
    }

    BOOL nearMode;
    INuiFrameTexture* pTexture;

    // Get the depth image pixel texture
    hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
        m_pDepthStreamHandle, &imageFrame, &nearMode, &pTexture);
    if (FAILED(hr))
    {
        goto ReleaseFrame;
    }

    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);
	//BYTE* depthD16 = new BYTE[LockedRect.size];
	memcpy(m_depthD16, LockedRect.pBits, LockedRect.size);
	//m_depthD16 = (USHORT*)depthD16;

    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {

        // Get the min and max reliable depth for the current frame
        int minDepth = (nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
        int maxDepth = (nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		USHORT * depthValue = depthValues;
		//USHORT * depthD16 = m_depthD16;

        const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);

        // end pixel is start + width*height - 1
        const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cDepthWidth * cDepthHeight);

        while ( pBufferRun < pBufferEnd )
        {
            // discard the portion of the depth that contains only the player index
            USHORT depth = pBufferRun->depth;
			//*depthD16 = depth;
			*depthValue = (depth >= minDepth && depth <= maxDepth ? depth - minDepth : 0);
			depthValue++;
			//depthD16++;

            // Increment our index into the Kinect's depth buffer
            ++pBufferRun;
        }
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);
	
    pTexture->Release();

ReleaseFrame:
    // Release the frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);


}


void KinectSensor::ProcessColor() {

	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame;

	// map color to depth

	// Get of x, y coordinates for color in depth space
	// This will allow us to later compensate for the differences in location, angle, etc between the depth and color cameras
	m_pNuiSensor->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
		NUI_IMAGE_RESOLUTION_640x480,
		NUI_IMAGE_RESOLUTION_640x480,
		kinectWidth*kinectHeight,
		m_depthD16,
		kinectWidth*kinectHeight * 2,
		m_colorCoordinates
	);

	// Attempt to get the color frame
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
	if (FAILED(hr))
	{
		return;
	}

	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data
	if (LockedRect.Pitch != 0)
	{

		BYTE* colorInfo = new BYTE[LockedRect.size];
		memcpy(colorInfo, LockedRect.pBits, LockedRect.size);


		for (LONG y = 0; y < kinectHeight; ++y)
		{
			LONG* pDest = (LONG*)(colorInfo + LockedRect.Pitch * y);
			for (LONG x = 0; x < kinectWidth; ++x)
			{

				// calculate index into depth array
				int depthIndex = x / m_colorToDepthDivisor + y / m_colorToDepthDivisor * kinectWidth;

				//// retrieve the depth to color mapping for the current depth pixel
				LONG colorInDepthX = m_colorCoordinates[depthIndex * 2];
				LONG colorInDepthY = m_colorCoordinates[depthIndex * 2 + 1];
				// make sure the depth pixel maps to a valid point in color space
				if (colorInDepthX >= 0 && colorInDepthX < kinectWidth && colorInDepthY >= 0 && colorInDepthY < kinectHeight)
				{
					// calculate index into color array
					LONG colorIndex = colorInDepthX + colorInDepthY * kinectWidth;

					// set source for copy to the color pixel
					LONG* pSrc = (LONG *)LockedRect.pBits + colorIndex;
					*pDest = *pSrc;
				}
				else
				{
					*pDest = 0;
				}

				pDest++;
			}
		}



		for (int j = 0; j < cDepthHeight; j++)
		{
			//unsigned char *pBuffer = (unsigned char*)(LockedRect.pBits) + j * LockedRect.Pitch;
			for (int i = 0; i < cDepthWidth; i++)
			{
				//int depthIndex = i / m_colorToDepthDivisor + j / m_colorToDepthDivisor * kinectWidth;

				//// retrieve the depth to color mapping for the current depth pixel
				//LONG colorInDepthX = m_colorCoordinates[depthIndex * 2];
				//LONG colorInDepthY = m_colorCoordinates[depthIndex * 2 + 1];
				//// make sure the depth pixel maps to a valid point in color space
				//if (colorInDepthX >= 0 && colorInDepthX < kinectWidth && colorInDepthY >= 0 && colorInDepthY < kinectHeight)
				{
					//内部数据是4个字节，0-1-2是BGR，第4个现在未使用
					//std::cout << (int)LockedRect.pBits[4 * (cDepthWidth * j + i)] << " " << (int)LockedRect.pBits[4 * (cDepthWidth * j + i) + 1] << " " << (int)LockedRect.pBits[4 * (cDepthWidth * j + i) + 2] << " " << (int)LockedRect.pBits[4 * (cDepthWidth * j + i) + 3] << std::endl;
					//std::cout << (unsigned short)LockedRect.pBits[4 * (cDepthWidth * j + i)] << " " << (unsigned short)LockedRect.pBits[4 * (cDepthWidth * j + i) + 1] << " " << (unsigned short)LockedRect.pBits[4 * (cDepthWidth * j + i) + 2] << " " << (unsigned short)LockedRect.pBits[4 * (cDepthWidth * j + i) + 3] << std::endl;
					//std::cout << (unsigned short)LockedRect.pBits[(LockedRect.Pitch * j + 4 * i)] << " " << (unsigned short)LockedRect.pBits[(LockedRect.Pitch * j + 4 * i) + 1] << " " << (unsigned short)LockedRect.pBits[(LockedRect.Pitch * j + 4 * i) + 2] << " " << (unsigned short)LockedRect.pBits[(LockedRect.Pitch * j + 4 * i) + 3] << std::endl;
					//std::cout << std::endl;
					colorsRGBValues[3 * (cDepthWidth * j + i) + 0] = (unsigned short)colorInfo[4 * (cDepthWidth * j + i) + 2]; // R
					colorsRGBValues[3 * (cDepthWidth * j + i) + 1] = (unsigned short)colorInfo[4 * (cDepthWidth * j + i) + 1]; // G
					colorsRGBValues[3 * (cDepthWidth * j + i) + 2] = (unsigned short)colorInfo[4 * (cDepthWidth * j + i) + 0]; // B
				}
			}
		}
		delete[] colorInfo;
	}
	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	// Release the frame
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);
}



/// <summary>
/// Handle new skeleton data
/// </summary>
void KinectSensor::ProcessSkeleton()
{
	NUI_SKELETON_FRAME skeletonFrame = { 0 };

	HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
	if (FAILED(hr))
	{
		return;
	}

	// smooth out the skeleton data
	m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);

	// Endure Direct2D is ready to draw
	//hr = EnsureDirect2DResources();
	if (FAILED(hr))
	{
		return;
	}

	//m_pRenderTarget->BeginDraw();
	//m_pRenderTarget->Clear();

	RECT rct;
	GetClientRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rct);
	int width = rct.right;
	int height = rct.bottom;

	for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;

		if (NUI_SKELETON_TRACKED == trackingState)
		{
			// We're tracking the skeleton, draw it
			//DrawSkeleton(skeletonFrame.SkeletonData[i], width, height);
		}
		else if (NUI_SKELETON_POSITION_ONLY == trackingState)
		{
			// we've only received the center point of the skeleton, draw that
			/*D2D1_ELLIPSE ellipse = D2D1::Ellipse(
				SkeletonToScreen(skeletonFrame.SkeletonData[i].Position, width, height),
				g_JointThickness,
				g_JointThickness
			);*/

			//m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
		}
	}

	//hr = m_pRenderTarget->EndDraw();

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
		//DiscardDirect2DResources();
	}
}


/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
void KinectSensor::SetStatusMessage(WCHAR * szMessage)
{
    SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
}