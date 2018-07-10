#pragma once

#include <iostream>

class Triangulator
{
	static const int        depthWidth = 640;
	static const int        depthHeight = 480;
	static const int        pointsPerPixel = 3;

public:
	/// <summary>
	/// Constructor
	/// </summary>
	Triangulator();

	/// <summary>
	/// Destructor
	/// </summary>
	~Triangulator();


	/// <summary>
	/// vertices
	/// </summary>
	uint32_t* indices;

	long indexCount;

	void triangulate(float* unorderedVertices);

//private:



};

