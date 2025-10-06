#pragma once
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_image.h"
#include <fstream>
#include <string>
#include <iostream>
using namespace std;

struct Vector2
{
	float x;
	float y;
};

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();
private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	//game
	SDL_Window* mWindow;
	SDL_Texture* newTexture;
	SDL_Texture* newTextureB;
	SDL_Renderer* mRenderer;
	Uint32 mTicksCount;
	bool mIsRunning;

	//bullet
	SDL_Texture* imgDart;
	string imageFileDart;
	int wD, hD;
	Vector2 mBulletPos;
	Vector2 mBulletVel;
	double degreesD;
	bool mShot;
	bool mBulletHit;
	bool mReload;
	bool bulletStatus;

	//ship
	SDL_Texture* imgClownL;
	string imageFileClownL;
	int wCL, hCL;
	SDL_Texture* imgClownR;
	string imageFileClownR;
	int wCR, hCR;
	Vector2 mShipPos;
	bool clownLeft;
	int mShipDir;

	//ball
	SDL_Texture* imgBloon;
	string imageFileBloon;
	int wBL, hBL;
	double degrees;
	SDL_RendererFlip flipType;
	Vector2 mBallPos;
	Vector2 mBallVel;
	bool bounce;
	int thicknessBall;
	int count;

	//background
	SDL_Texture* imgBckGrnd;
	string imageFileBckGrnd;
	int wBk, hBk;

	//text
	TTF_Font* theFont;
	SDL_Color textColor;
	SDL_Surface* textSurface;
	SDL_Surface* textSurfaceB;
	string textureText;
	string textureTextB;
	ofstream outFile;
	ifstream inFile;
	int score;
	int bestScore;
	int mWidth;
	int mHeight;
	int mWidthB;
	int mHeightB;
	int scoreX;
	int scoreY;
	int scoreBY;

	//sounds
	Mix_Chunk* boing;
	Mix_Chunk* hit;
	Mix_Chunk* pop;
	Mix_Chunk* Dthrow;
	Mix_Music* music;
};
