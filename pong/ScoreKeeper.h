#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <iostream>
#include <string>

#define ROUNDRESTTIME 2.0f
#define GAMERESTTIME  5.0f

class ScoreKeeper
{
private:
	unsigned lScore = 0, rScore = 0;
	GLuint lScoreText, rScoreText;
	float* currTime;
	float* anchorTime;
	int* gameState;

public:
	ScoreKeeper(GLuint LST, GLuint RST, float* ct, float* at, int* gs)
		: lScoreText(LST)
		, rScoreText(RST)
		, currTime(ct)
		, anchorTime(at)
		, gameState(gs)
	{
	}

	// change scores to zero and update textures
	void resetGame()
	{
		this->lScore = 0;
		this->rScore = 0;

		int width, height, nrChannels;
		unsigned char* zero = stbi_load("scoreImages/0.png", &width, &height, &nrChannels, 0);

		glBindTexture(GL_TEXTURE_2D, this->lScoreText);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, zero);
		glBindTexture(GL_TEXTURE_2D, this->rScoreText);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, zero);

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(zero);
	}

	// input true if the left paddle scored, false if the right paddle scored
	void score(bool leftSide)
	{
		if (leftSide && this->lScore >= 5)
		{
			*this->gameState = 1;
			*this->anchorTime = *this->currTime + GAMERESTTIME;
			this->resetGame();
		}
		else if (!leftSide && this->rScore >= 5)
		{
			*this->gameState = 2;
			*this->anchorTime = *this->currTime + GAMERESTTIME;
			this->resetGame();
		}
		else
		{
			*this->gameState = 0;
			if (leftSide)
			{
				this->lScore++;

				int width, height, nrChannels;
				unsigned char* data = stbi_load((std::string("scoreImages/") + std::to_string(this->lScore) + std::string(".png")).c_str(), &width, &height, &nrChannels, 0);

				if (!data)
					std::cout << "failed to load image" << std::endl;

				glBindTexture(GL_TEXTURE_2D, this->lScoreText);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

				stbi_image_free(data);
			}
			else
			{
				this->rScore++;

				int width, height, nrChannels;
				unsigned char* data = stbi_load((std::string("scoreImages/") + std::to_string(this->rScore) + std::string(".png")).c_str(), &width, &height, &nrChannels, 0);

				if (!data)
					std::cout << "failed to load image" << std::endl;

				glBindTexture(GL_TEXTURE_2D, this->rScoreText);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

				stbi_image_free(data);
			}
			*this->anchorTime = *this->currTime + ROUNDRESTTIME;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
};