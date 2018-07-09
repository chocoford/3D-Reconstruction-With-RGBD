//------------------------------------------------------------------------------
// <copyright file="DepthBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "NuiApi.h"

class CDepthBasics
{
    static const int        cDepthWidth  = 640;
    static const int        cDepthHeight = 480;
    static const int        cBytesPerPixel = 4;

    static const int        cStatusMessageMaxLen = MAX_PATH*2;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    CDepthBasics();

    /// <summary>
    /// Destructor
    /// </summary>
    ~CDepthBasics();

    /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    /// <param name="hInstance"></param>
    /// <param name="nCmdShow"></param>
    int                     Run();

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

private:
    HWND                    m_hWnd;

    bool                    m_bNearMode;

    // Current Kinect
    INuiSensor*             m_pNuiSensor;
    
    HANDLE                  m_pDepthStreamHandle;
    HANDLE                  m_hNextDepthFrameEvent;

    /// <summary>
    /// Handle new depth data
    /// </summary>
    void                    ProcessDepth();

    /// <summary>
    /// Set the status bar message
    /// </summary>
    /// <param name="szMessage">message to display</param>
    void                    SetStatusMessage(WCHAR* szMessage);
};
