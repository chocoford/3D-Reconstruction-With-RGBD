//------------------------------------------------------------------------------
// <copyright file="DepthBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "NuiApi.h"
#include "header.h"

class KinectSensor
{
    static const int        cDepthWidth  = kinectWidth;
    static const int        cDepthHeight = kinectHeight;
    static const int        cBytesPerPixel = 4;

    static const int        cStatusMessageMaxLen = MAX_PATH*2;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    KinectSensor();

    /// <summary>
    /// Destructor
    /// </summary>
    ~KinectSensor();

	/// <summary>
	/// Main processing function
	/// </summary>
	void                    Update();

	/// <summary>
	/// Create the first connected Kinect found 
	/// </summary>
	/// <returns>S_OK on success, otherwise failure code</returns>
	HRESULT                 CreateFirstConnected();

	void DisConnected();

	BYTE*                   m_depthRGBX;
	USHORT*                 depthValues;
	unsigned short*                 colorsRGBValues;

private:
    HWND                    m_hWnd;

    bool                    m_bNearMode;

    // Current Kinect
    INuiSensor*             m_pNuiSensor;
    
    HANDLE                  m_pDepthStreamHandle;
	HANDLE                  m_pColorStreamHandle;
    HANDLE                  m_hNextDepthFrameEvent;
	HANDLE                  m_hNextColorFrameEvent;
	HANDLE                  m_pSkeletonStreamHandle;
	HANDLE                  m_hNextSkeletonEvent;

	USHORT*                             m_depthD16;
	LONG*                               m_colorCoordinates;
	LONG                                m_colorToDepthDivisor;

    /// <summary>
    /// Handle new depth and color data
    /// </summary>
    void                    ProcessDepth();
	void                    ProcessColor();
	void                    ProcessSkeleton();

    /// <summary>
    /// Set the status bar message
    /// </summary>
    /// <param name="szMessage">message to display</param>
    void                    SetStatusMessage(WCHAR* szMessage);
};
