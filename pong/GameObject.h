#pragma once

struct GameObject2D
{
	const float height, width, rootSpeedX, rootSpeedY;

	float xDist, yDist, speedX, speedY;
	float left, right, top, bottom;

	// central x & y coords, h and w are TOTAL height and width. both sides combined. sX and sY are x and y speeds respectively.
	GameObject2D(float x, float y, float h, float w, float sX, float sY)
		: xDist(x)
		, yDist(y)
		, height(h)
		, width(w)
		, speedX(sX)
		, speedY(sY)
		, rootSpeedX(sX)
		, rootSpeedY(sY)
	{
		this->left = this->xDist - (this->width / 2.0f);
		this->right = this->xDist + (this->width / 2.0f);
		this->top = this->yDist + (this->height / 2.0f);
		this->bottom = this->yDist - (this->height / 2.0f);
	}

	void update()
	{
		this->left = this->xDist - (this->width / 2.0f);
		this->right = this->xDist + (this->width / 2.0f);
		this->top = this->yDist + (this->height / 2.0f);
		this->bottom = this->yDist - (this->height / 2.0f);
	}
};