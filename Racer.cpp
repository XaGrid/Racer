#include <conio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

struct Rect{
	int x;
	int y;
	int width;
	int height;
};

struct Sprite{
	int width;
	int height;
	char* spriteString;
};

const Sprite playerCarSprite = {5 , 5 , "|===| /0\\  |o| /owo\\0===0"};
const Sprite enemyCarSprite = {5 , 4 , "0===0\\owo/ \\o/  0=0 "};

class Car{
	public:
		Rect carRect;

		Car(){
			setSprite(playerCarSprite);
			carRect.x = 0;
			carRect.y = carRect.height;
			speed = 1;
		}

		Car(int x , int y , bool enemy){
			if (enemy){
				setSprite(enemyCarSprite);
			} else {
				setSprite(playerCarSprite);
			}
			carRect.x = x;
			carRect.y = y;
			speed = 1;
		}

		void setSprite(Sprite sprite){
			carRect.width = sprite.width;
			carRect.height = sprite.height;
			carSprite = new char[sprite.width * sprite.height];
			memcpy((void*) carSprite , (void*) sprite.spriteString , sprite.width * sprite.height);			
		}
		
		char getSpritePixel(int x , int y){
			return carSprite[x + (carRect.height - 1 - y) * carRect.width];
		}

		void moveRight(){
			carRect.x += speed;
		}

		void moveLeft(){
			carRect.x -= speed;
		}

		void moveUp(){
			carRect.y += speed;
		}

		void moveDown(){
			carRect.y -= speed;
		}

		void setSpeed(int carSpeed){
			speed = carSpeed;
		}

		int getSpeed(){
			return speed;
		}

		int getX(){
			return carRect.x;
		}

		int getY(){
			return carRect.y;
		}

		int getWidth(){
			return carRect.width;
		}

		int getHeight(){
			return carRect.height;
		}

		Rect getRect(){
			return carRect;
		}

		bool rectCollision(Rect rectangle){
			return (
		        (carRect.x < rectangle.x + rectangle.width)&&
		        (carRect.x + carRect.width > rectangle.x)&&
		        (carRect.y < rectangle.y + rectangle.height)&&
		        (carRect.y + carRect.height > rectangle.y)
		    );
		}

	private:
		char *carSprite;
		int speed;
};

class Game{
	public:
		Game(int width , int height){
			outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			consoleWindow = GetConsoleWindow();
			consoleInfo.dwSize = 100;
			consoleInfo.bVisible = false;
			zeroCoords = {0 , 0};

			RUNNING = true;
			score = 0;
			msPerFrame = 13;
			boostKeyDown = false;

			lastCarSpawn = 0;
			carSpawnInterval = 10;

			gameUpdateInterval = 10;
			gameUpdateCounter = 0;

			initDisplay(width , height);
			system("cls");
		}
		
		void initDisplay(int width , int height){
			displayWidth = width;
			displayHeight = height;
			displayContent = new char[width * (height + 1) + 1];

			for (int i = width; i < height * (width + 1); i += width + 1){
				displayContent[i] = '\n';
			}

			displayContent[width * (height + 1)] = '\0';
		}
		
		void clearDisplay(){
			for (int y = 0; y < displayHeight; y++){
				for (int x = 0; x < displayWidth; x++){
					displayContent[x + y * (displayWidth + 1)] = ' ';
				}
			}		
		}
		
		void setPixel(int x , int y , char pixel){
			if (x >= 0 && x < displayWidth && y >= 0 && y < displayHeight){
				displayContent[x + (displayHeight - 1 - y) * (displayWidth + 1)] = pixel;
			}
		}
		
		void renderCar(Car &carToDraw){
			for (int y = 0; y < carToDraw.getHeight(); y++){
				for (int x = 0; x < carToDraw.getWidth(); x++){
					setPixel(x + carToDraw.getX() , carToDraw.getY() - (carToDraw.getHeight() - y) , carToDraw.getSpritePixel(x , y));
				}
			}		
		}
		
		void renderDisplay(){
			clearDisplay();
			renderCar(playerCar);
			for (int i = 0; i < enemyCars.size(); i++){
				renderCar(enemyCars[i]);
			}
		}
		
		void drawDisplay(){
			SetConsoleCursorPosition(outputHandle , zeroCoords);
			cout << displayContent;
		}

		void spawnEnemyCar(){
			if (lastCarSpawn <= 0){
				enemyCars.push_back(Car(rand() % (displayWidth - 5) , displayHeight + 4 , true));
				lastCarSpawn = carSpawnInterval;
			}
			lastCarSpawn--;
		}

		void carsCollision(){
			for (int i = 0; i < enemyCars.size(); i++){
				if (playerCar.rectCollision(enemyCars[i].carRect)){
					RUNNING = false;
				}			
			}
		}

		void carsMoving(){
			for (int i = 0; i < enemyCars.size(); i++){
				enemyCars[i].moveDown();
				if (enemyCars[i].getY() < 0){
					enemyCars.erase(enemyCars.begin() + i);
					i--;
					score++;
					updateDifficulty();
				}
			}
		}

		void updateDifficulty(){
			switch (score){
				case (10):{
					gameUpdateInterval = 8;
					carSpawnInterval = 9;
					break;
				}
				case (25):{
					gameUpdateInterval = 6;
					carSpawnInterval = 9;
					break;
				}
				case (40):{
					gameUpdateInterval = 4;
					carSpawnInterval = 8;
					break;
				}
				case (55):{
					gameUpdateInterval = 2;
					carSpawnInterval = 7;
					break;
				}
				case (75):{
					gameUpdateInterval = 1;
					carSpawnInterval = 6;
					break;
				}
				case (200):{
					gameUpdateInterval = 0;
					break;
				}
			}
		}
		
		void updateGame(){
			carsCollision();
			if (gameUpdateCounter < 0){
				spawnEnemyCar();
				carsMoving();
				gameUpdateCounter = gameUpdateInterval;
			}
			if (boostKeyDown){
				gameUpdateCounter -= 3;
			} else {
				gameUpdateCounter -= 1;			
			}
		}

		void handleInput(){
			if (GetAsyncKeyState(65)){
				if (playerCar.getX() - playerCar.getSpeed() >= 0){
					playerCar.moveLeft();
				}
			}

			if (GetAsyncKeyState(68)){
				if (playerCar.getWidth() + playerCar.getX() + playerCar.getSpeed() <= displayWidth){
					playerCar.moveRight();
				}
			}
			
			if (GetAsyncKeyState(32)){
				if (!boostKeyDown){
					boostKeyDown = true;
				}
			} else {
				if (boostKeyDown){
					boostKeyDown = false;
				}
			}
			
		}

		void gameLoop(){
			while (RUNNING){
				lastFrameTime = clock();
				SetConsoleCursorInfo(outputHandle, &consoleInfo);

				if (GetForegroundWindow() == consoleWindow){
					handleInput();
				}

				updateGame();
				renderDisplay(); 
				drawDisplay();

				while (clock() - lastFrameTime < msPerFrame){
					Sleep(1);
				}
				cout << "Score: " << score << endl; 
			}
		}
		
	private:
		bool RUNNING;
		int lastFrameTime;
		int msPerFrame;

		COORD zeroCoords;
		HANDLE outputHandle;
		HWND consoleWindow;
		CONSOLE_CURSOR_INFO consoleInfo;

		char *displayContent;
		int displayHeight;
		int displayWidth;

		bool boostKeyDown;

		int lastCarSpawn;
		int carSpawnInterval;

		int gameUpdateInterval;
		int gameUpdateCounter;

		int score;

		Car playerCar;
		vector<Car> enemyCars;
};

int main(){

	srand(time(NULL));
	int levelSize;

	while (1){
		cout << "A - move left , D - move right , Space - speed up" << endl;
		cout << "Select level size:" << endl;
		cout << "1 = 80x25" << endl;
		cout << "2 = 100x35" << endl;
		cout << "3 = 150x40" << endl;
		cout << "4 = input custom level size" << endl;
		
		cin >> levelSize;
		
		switch (levelSize){
			case (1):{
				Game gm(80 , 25);
				gm.gameLoop();
				break;
			}
			case (2):{
				Game gm(100 , 35);			
				gm.gameLoop();
				break;
			}
			case (3):{
				Game gm(150 , 40);
				gm.gameLoop();
				break;
			}
			case (4):{
				int w , h;
				cout << "Input width and height: ";
				cin >> w >> h;
				Game gm(w , h);
				gm.gameLoop();
				break;				
			}
			default:{
				cout << "Wrong level select" << endl;
				break;
			}	
		}

		system("pause");
		system("cls");
	}


	return 0;

}

