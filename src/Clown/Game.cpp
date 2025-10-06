
#include "Game.h"

//add start and end screens

const int thicknessWall = 15;
const int thicknessShip = 60; 
const float ShipH = 65.0f;
using namespace std;


Game::Game()
{
	//misc
	mWindow=nullptr;
	newTexture = NULL;
	mRenderer=nullptr;

	mTicksCount=0;
	mIsRunning=true;
	score = 0;
	mShipDir=0;

	//img variables
	imgDart = nullptr;
	imgClownL = nullptr;
	imgBloon = nullptr;
	imgBckGrnd = nullptr;

	flipType = SDL_FLIP_NONE;
	degrees = 0;
	degreesD = 0;

	//Bullet variable
	mShot = false;
	mBulletHit = true;
	mReload = true;
	bulletStatus = true;

	//balloon variables
	bounce = false;

	//clown state 
	clownLeft = true;

	//sounds
	boing = NULL;
	music = NULL;
	pop = NULL;
	Dthrow = NULL;
	hit = NULL;
}

bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	// Initialize SDL_mixer after SDL
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
		return false;
	}

	mWindow = SDL_CreateWindow("Clown Balloon", 100, 100, 1024, 768, 0);
	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}

	//font
	TTF_Init();
	theFont = TTF_OpenFont("Dubai-Regular.ttf", 28);
	textColor = { 255, 255, 255 };
	inFile.open("bestScore.txt");
	inFile >> bestScore;
	inFile.close();

	//dart
	imageFileDart = "Dart.png";
	imgDart = IMG_LoadTexture(mRenderer, imageFileDart.c_str());
	SDL_QueryTexture(imgDart, NULL, NULL, &wD, &hD);

	//Clown 
	imageFileClownL = "ClownLeft.png";
	imgClownL = IMG_LoadTexture(mRenderer, imageFileClownL.c_str());
	SDL_QueryTexture(imgClownL, NULL, NULL, &wCL, &hCL);
	imageFileClownR = "ClownRight.png";
	imgClownR = IMG_LoadTexture(mRenderer, imageFileClownR.c_str());
	SDL_QueryTexture(imgClownR, NULL, NULL, &wCR, &hCR);

	//Balloon
	imageFileBloon = "Balloon.png";
	imgBloon = IMG_LoadTexture(mRenderer, imageFileBloon.c_str());

	//Background
	imageFileBckGrnd = "Background.png";
	imgBckGrnd = IMG_LoadTexture(mRenderer, imageFileBckGrnd.c_str());
	SDL_QueryTexture(imgBckGrnd, NULL, NULL, &wBk, &hBk);
	
	//set clown
	mShipPos.x = 1024.0 / 2.0f;
	mShipPos.y = 768.0f - thicknessShip;

	//set Balloon
	mBallPos.x = 1024.0f/2;
	mBallPos.y = 768.0f/2;
	mBallVel.x = 175.0f;
	mBallVel.y = -125.0f;
	//balloon thickness is not a constant
	thicknessBall = 125.0f;

	//Set Dart
	mBulletPos.x = mShipPos.x;
	mBulletPos.y = mShipPos.y;
	mBulletVel.x = 0.0f;
	mBulletVel.y = 0.0f;

	//set noises 
	boing = Mix_LoadWAV("Spring-Boing.wav");

	pop = Mix_LoadWAV("pop.wav");

	Dthrow = Mix_LoadWAV("Dthrow.wav");

	hit = Mix_LoadWAV("hit.wav");

	//set music
	music = Mix_LoadMUS("music.wav");
	Mix_VolumeMusic(60);
	Mix_PlayMusic(music, -1);
	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		//set music running and z-key to pause/play 
		if (event.type == SDL_QUIT)
		{
			mIsRunning = false;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_z:
				if (Mix_PlayingMusic() == 0)
				{
					Mix_PlayMusic(music, -1);
				}
				else
				{
					if (Mix_PausedMusic() == 1)
					{
						Mix_ResumeMusic();
					}
					else
					{
						Mix_PauseMusic();
					}
				}
				break;
			}
		}
	}

	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
	mShipDir = 0;
	if (state[SDL_SCANCODE_RIGHT])
	{
		mShipDir += 1;
		clownLeft = false;
	}
	if (state[SDL_SCANCODE_LEFT])
	{
		clownLeft = true;
		mShipDir -= 1;
	}
	if (state[SDL_SCANCODE_UP])
	{
		//check if bullet can be shot then shoot
		if (mBulletHit == true && mReload == true && mShot == false && bulletStatus == true)
		{
			Mix_PlayChannel(-1, Dthrow, 0);
			mShot = true;
			mBulletHit = false;
			mReload = false;
			bulletStatus = false;
		}
	}
	if (state[SDL_SCANCODE_R])
	{
		//check if bullet was shot and has hit then reload 
		if (mBulletHit == true && mReload == false && mShot == false && bulletStatus == false)
		{
			mShot = false;
			mBulletHit = false;
			mReload = true;
			bulletStatus = true;
		}
	}
}

void Game::UpdateGame()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	mTicksCount = SDL_GetTicks();
	//if ship is moving 
	if (mShipDir != 0 )
	{
		//move shipt left and right 
		mShipPos.x += mShipDir * 300.0f * deltaTime;
		//keep ship in bounds
		if (mShipPos.x < thicknessWall)
		{
			mShipPos.x = thicknessWall;
		}
		else if (mShipPos.x > 1024.0f-(thicknessShip+thicknessWall))
		{
			mShipPos.x = 1024.0f - (thicknessShip + thicknessWall);
		}
	}

	//if bullet hasn't shot
	if (mShot == false)
	{
		//move bullet with ship
		mBulletPos.x = mShipPos.x;
	}
	else 
	{
		//move bullet up
		mBulletPos.y -= 5 * 300.0f * deltaTime;
	}
	
	//if balloon was hit and now bouncing 
	if (bounce == true) 
	{
		//increase timer count
		count++;
		//while count != 100 the balloon continues to rise
		if (count == 100) 
		{
			mBallVel.y *= -1;
			bounce = false; 
			count = 0;
		}
	}
	//move balloon
	mBallPos.y += mBallVel.y * deltaTime;
	mBallPos.x += mBallVel.x * deltaTime;

	//find x and y difference between ship and ball
	float diffBSy = mShipPos.y - mBallPos.y;
	float diffBSx = mShipPos.x - mBallPos.x;
	//make x and y difference abs val
	diffBSy = (diffBSy > 0.0f) ? diffBSy : -diffBSy;
	diffBSx = (diffBSx > 0.0f) ? diffBSx : -diffBSx;
	//if balloon hit ship 
	if (diffBSy <= thicknessBall / 2.0f && diffBSx <= thicknessBall/2.0f)
	{
		//game over!
		mIsRunning = false;
	}

	//bullet and ball collison 
	float diffBBy = mBulletPos.y - mBallPos.y;
	float diffBBx = mBulletPos.x - (mBallPos.x + 20);
	diffBBy = (diffBBy > 0.0f) ? diffBBy : -diffBBy;
	diffBBx = (diffBBx > 0.0f) ? diffBBx : -diffBBx;
	if (diffBBy <= thicknessBall / 2.0f  && diffBBx <= thicknessBall )
	{
		//if bullet shot 
		if (bulletStatus == false) 
		{
			//increase score, ball size, bounce
			score++;
			thicknessBall -= 25;
			if (thicknessBall == 25) 
			{
				//increase my balloons speed everytime it pops
				mBallVel.x *= 1.12f;
				mBallVel.y *= 1.02f;
				//set back to normal size
				thicknessBall = 100;
				Mix_PlayChannel(-1, pop, 0);
			}
			if (score > bestScore)
			{
				bestScore = score;
				outFile.open("bestScore.txt");
				outFile << bestScore;
				outFile.close();
			}
			Mix_PlayChannel(-1, hit, 0);
			if (mBallVel.y > 0) 
			{
				//if my ball falling
				mBallVel.y *= -1;
			}
			bounce = true; 

			mShot = false;
			mBulletHit = true;
			
			//move bullet off screen after hit
			mBulletPos.x = -1000;
			mBulletPos.y = -1000;
		}
	}

	//keep balloon in bounds 
	if (mBallPos.x <= 0)
	{
		//if ballon hits left wall
		if (mBallVel.x < 0)
			//if moving left move right
			mBallVel.x *= -1.0f;
		Mix_PlayChannel(-1, boing, 0);
	}
	else if (mBallPos.x >= 1024.0f -thicknessBall && mBallVel.x > 0.0f)
	{
		//right wall
		if (mBallVel.x > 0)
			mBallVel.x *= -1.0f;
		Mix_PlayChannel(-1, boing, 0);
	}
	
	if (mBallPos.y <= thicknessBall/2.0f && mBallVel.y < 0.0f)
	{
		//top wall 
		if (mBallVel.y < 0)
			mBallVel.y *= -1.0f;
		//cancel bounce 
		bounce = false; 
		count = 0;
		Mix_PlayChannel(-1, boing, 0);
	}
	else if (mBallPos.y >= (768 - thicknessWall - thicknessBall) && mBallVel.y > 0.0f)
	{
		//ground, game over
		mIsRunning = false;
	}

	//check if dart hit top wall
	if (mBulletPos.y <= thicknessWall)
	{
		mBulletHit = true;
		mShot = false;
		//move bullet off screen
		mBulletPos.x = -1000;
		mBulletPos.y = -1000;
	}
	if (mReload == true)
	{
		//if reload move dart to clown
		mBulletPos.x = mShipPos.x + thicknessWall;
		mBulletPos.y = mShipPos.y;
	}
}

void Game::GenerateOutput()
{
	SDL_Rect background;
	background.x = 0;
	background.y = 0;
	background.w = 1024.0f;
	background.h = 768.0f;
	SDL_RenderCopy(mRenderer, imgBckGrnd, NULL, &background);

	SDL_Rect ship;
	ship.x = mShipPos.x;
	ship.y = mShipPos.y - thicknessWall;
	ship.w = thicknessShip;
	ship.h = ShipH;
	//face clown in correct direction
	if (clownLeft == true)
	{
		SDL_RenderCopy(mRenderer, imgClownL, NULL, &ship);
	}
	else 
	{
		SDL_RenderCopy(mRenderer, imgClownR, NULL, &ship);
	}

	SDL_Rect* clipB = NULL;
	SDL_Rect ball = { mBallPos.x, mBallPos.y , thicknessBall, thicknessBall};
	if (clipB != NULL)
	{
		ball.w = clipB->w;
		ball.h = clipB->h;
	}
	//rotate balloon
	if (mBallVel.x > 0 && mBallVel.y < 0) 
	{
		degrees = 45;
	}
	else if (mBallVel.x > 0 && mBallVel.y > 0)
	{
		degrees = 135;
	}
	else if (mBallVel.x < 0 && mBallVel.y > 0)
	{
		degrees = -135;
	}
	else if (mBallVel.x > 0 && mBallVel.y < 0)
	{
		degrees = 45;
	}
	else 
	{
		degrees = 0;
	}
	SDL_RenderCopyEx(mRenderer, imgBloon, clipB, &ball, degrees, NULL, flipType);



	SDL_Rect* clipD = NULL;
	SDL_Rect bullet = { mBulletPos.x, mBulletPos.y , thicknessWall * 2, thicknessShip / 2 };
	if (clipD != NULL)
	{
		bullet.w = clipD->w;
		bullet.h = clipD->h;
	}
	//rotate dart when thrown
	if (mShot == true) 
	{
		degreesD = 45;
	}
	else 
	{
		degreesD = 0;
	}
	SDL_RenderCopyEx(mRenderer, imgDart, clipD, &bullet, degreesD, NULL, flipType);


	//score
	textureText = "Score: " + to_string(score);
	textSurface = TTF_RenderText_Solid(theFont, textureText.c_str(), textColor);
	newTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
	mWidth = textSurface->w;
	mHeight = textSurface->h;
	scoreX = (1024 - (mWidth * 2));
	scoreY = mHeight;
	SDL_Rect* clip2 = NULL;
	SDL_Rect renderQuad = { scoreX, scoreY, mWidth, mHeight };
	SDL_RenderCopyEx(mRenderer, newTexture, clip2, &renderQuad, 0, NULL, SDL_FLIP_NONE);

	//best score
	textureTextB = "Best Score: " + to_string(bestScore);
	textSurfaceB = TTF_RenderText_Solid(theFont, textureTextB.c_str(), textColor);
	newTextureB = SDL_CreateTextureFromSurface(mRenderer, textSurfaceB);
	mWidthB = textSurfaceB->w;
	mHeightB = textSurfaceB->h;
	scoreBY = (mHeightB * 2);
	SDL_Rect* clipBe = NULL;
	SDL_Rect renderQuadB = { scoreX, scoreBY, mWidthB, mHeightB };
	SDL_RenderCopyEx(mRenderer, newTextureB, clipBe, &renderQuadB, 0, NULL, SDL_FLIP_NONE);

	SDL_RenderPresent(mRenderer);
}

void Game::Shutdown()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}
