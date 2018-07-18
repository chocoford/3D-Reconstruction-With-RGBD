# 基于Kinect-RGBD的三维重建及简单应用

![](https://img.shields.io/badge/build-passing-brightgreen.svg)
![](https://img.shields.io/badge/visual_studio-2017-blue.svg)
![](https://img.shields.io/badge/docs-not_ready-red.svg)
![](https://img.shields.io/badge/Kinect_SDK-v1.8.0-brightgreen.svg)
![](https://img.shields.io/badge/BUCT-cs2015-green.svg)
![](https://img.shields.io/badge/license-MIT-lightgray.svg)
![](https://img.shields.io/badge/platform-Windows_10-lightgray.svg)

## 介绍
这是一个在北京化工大学2018年第三学期编写的小demo，是一个由*胡伟*老师委派给我们的任务--运用Microsoft Kinect编写简单的小程序。因此我们用Kinect做了一个简单的三维重建。这个demo还未完成，项目里面还有许多写了一半或者为某些本想实现的功能编写的代码。在这个demo里，我们实现了4种呈现由点云构建出来平面的显示模式--根据距离变换颜色的显示模式、法向量可视化显示模式、简易光影显示模式。并且实现了简单的AR应用。
![](Resource/AR_show.PNG)

## 功能特性

- [x] Kinect与OpenGL的连接.
- [x] 点云构建.
- [x] 由点云构建平面.
- [x] 距离显示模式.
- [x] 法向量显示模式.
- [x] 光影显示模式.
- [x] 颜色显示模式.
- [ ] 增强现实应用(还未完成).
- [ ] 完整的文档

## 需求

* Windows 10 操作系统
* Microsoft Visual Studio 2017(版本可能无关紧要)
* Kinect SDK v1.8
* OpenGL v3.3
	* glfw3
	* glad

## 如何开始使用代码

#### 设置项目属性

1. 包含目录:
	
	![](Resource/include_path.PNG)
	
2. 引用目录:

	![](Resource/library_path.PNG)
3. 附加依赖项:
	
    	kernel32.lib
		user32.lib
		winspool.lib
		comdlg32.lib
		advapi32.lib
		shell32.lib
		ole32.lib
		oleaut32.lib
		uuid.lib
		odbc32.lib
		odbccp32.lib
		Kinect10.lib
		glfw3.lib
		opengl32.lib

## 下一步
如果我有时间，我将会继续完成这个demo，直到它能够像一个像样的产品一样。但是由于我目前的首要任务是准备考研，因此理论上最近的一次更新将会在半年以后。或者是永远。
## 许可

基于Kinect-RGBD的三维重建及简单应用 is released under the MIT license. See LICENSE for details.
