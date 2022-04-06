#pragma once

//#include<iostream>
//#include"bitmap.h"

//unsigned int loadCubemap(const char* filename[])
//{
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_PROXY_TEXTURE_CUBE_MAP, textureID);
//	unsigned char* pxls[6];
//	BITMAPINFOHEADER info[6];
//	BITMAPFILEHEADER file[6];
//	for (int i = 0;i < 6;i++) 
//	{ 
//		loadbitmap(filename[i], pxls[i], &info[i], &file[i]);
//		if (pxls != NULL) 
//		{ 
//			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, info[i].biWidth, info[i].biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pxls[i]);
//		}
//		delete[]pxls[i];
//	}
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//	return textureID;
//}