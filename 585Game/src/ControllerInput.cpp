#pragma once
#include <Boilerplate/Window.h>
#include <vector>
#define INPUT_DEADZONE 7849

std::vector<ConStruct> controller_data;


int button_array[14]= { 1,2,4,8,16,32,64,128,256,512,4096,8192,16384,32768 };
bool flag = true;
DWORD threadids[4];
std::vector<HANDLE> threadhandles;
XboxInput::XboxInput() {
	
}

void XboxInput::run()
{
	controller_data.push_back(ConStruct());
	int p0;
	p0 = 0;
	threadhandles.push_back(CreateThread(NULL, 0, update_controller_thread, &p0, NULL, &threadids[0]));
}

void XboxInput::run(int numPlayers)
{
	int p1, p2, p3;
	p1 = 1; p2 = 2; p3 = 3;
	//std::thread controller_input_thread(update_controller_thread,NULL);
	for (int i = 1; i < numPlayers; i++) {
		controller_data.push_back(ConStruct());
		if (i == 1) threadhandles.push_back(CreateThread(NULL, 0, update_controller_thread, &p1, NULL, &threadids[1]));
		else if (i == 2) threadhandles.push_back(CreateThread(NULL, 0, update_controller_thread, &p2, NULL, &threadids[2]));
		else if (i == 3) threadhandles.push_back(CreateThread(NULL, 0, update_controller_thread, &p3, NULL, &threadids[3]));
	}
}

void XboxInput::stop() {
	flag = false;
	for (int i = 0; i < threadhandles.size(); i++) {
		WaitForSingleObject(threadhandles[i], INFINITE);
		CloseHandle(threadhandles[i]);
	}
}

void XboxInput::update() {
	for (int i = 0; i < controller_data.size(); i++) {
		data[i] = controller_data[i];
	}
}




DWORD WINAPI update_controller_thread(LPVOID lpParameter) {
	int value = *(int*)lpParameter;
	int time_before = 0;
	int time_after = 0;
	int time_diff = 0;
	while (flag) {
		//check if the controller is connected or not
		//DWORD dwResult;
		//for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {  //-----this will be useful when multi controller connected
		XINPUT_STATE state;
		//SecureZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		DWORD dwResult = XInputGetState(value, &state); //assume only one Xbox controller is connected
		time_diff = time_after - time_before;
		if (time_diff >= 5) {  // CHECK IF controller is connected every 5 seconds ?
			time_before = static_cast<int>(time(NULL));

			if (dwResult == ERROR_SUCCESS)
			{
				std::cout << "Controller " << value << " connected" << std::endl;
			}
			else
			{
				std::cout << "Controller " << value << " disconnected!" << std::endl;
			}
			//}

			time_after = static_cast<int>(time(NULL));
		}
		else {
			time_after = static_cast<int>(time(NULL));
		}
		if (dwResult == ERROR_SUCCESS) {
			checkLeftThumbBar(state, value);
			checkRightThumbBar(state, value);
			checkButtons(state, value);
		}
		//}
	}
	return 0;
}
void checkLeftThumbBar(XINPUT_STATE state, int cNum) {
	float LX = state.Gamepad.sThumbLX; //this one is checking for left thumb bar
	float LY = state.Gamepad.sThumbLY;

	//determine how far the controller is pushed
	float magnitude = sqrt(LX * LX + LY * LY);

	//determine the direction the controller is pushed
	float normalizedLX = LX / magnitude;
	float normalizedLY = LY / magnitude;

	float normalizedMagnitude = 0;

	//check if the controller is outside a circular dead zone
	if (magnitude > INPUT_DEADZONE)
	{
		//clip the magnitude at its expected maximum value
		if (magnitude > 32767) magnitude = 32767;

		//adjust magnitude relative to the end of the dead zone
		magnitude -= INPUT_DEADZONE;

		//optionally normalize the magnitude with respect to its expected range
		//giving a magnitude value of 0.0 to 1.0
		normalizedMagnitude = magnitude / (32767 - INPUT_DEADZONE);
	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		magnitude = 0.0;
		normalizedMagnitude = 0.0;
		normalizedLX = 0.0; // each controller has different initial setting, and seems not to be 0
		normalizedLY = 0.0; // since user doesn't push the bar, we just manually set it to 0.
	}
	controller_data[cNum].LThumb_magnitude = normalizedMagnitude;
	controller_data[cNum].LThumb_X_direction = normalizedLX;
	controller_data[cNum].LThumb_Y_direction = normalizedLY;
	if (normalizedMagnitude > 0) { // FOR debug use, only shows the status when we push the bar
		//std::cout << "magnitude is:" << normalizedMagnitude << std::endl;
		//std::cout << "left bar direction in x axis is:" << normalizedLX << std::endl;
		//std::cout << "left bar direction in y axis is:" << normalizedLY << std::endl;
		//repeat for right thumb stick
	}

}
void checkRightThumbBar(XINPUT_STATE state, int cNum) {
	float RX = state.Gamepad.sThumbRX; //this one is checking for left thumb bar
	float RY = state.Gamepad.sThumbRY;

	//determine how far the controller is pushed
	float magnitude = sqrt(RX * RX + RY * RY);

	//determine the direction the controller is pushed
	float normalizedRX = RX / magnitude;
	float normalizedRY = RY / magnitude;

	float normalizedMagnitude = 0;

	//check if the controller is outside a circular dead zone
	if (magnitude > INPUT_DEADZONE)
	{
		//clip the magnitude at its expected maximum value
		if (magnitude > 32767) magnitude = 32767;

		//adjust magnitude relative to the end of the dead zone
		magnitude -= INPUT_DEADZONE;

		//optionally normalize the magnitude with respect to its expected range
		//giving a magnitude value of 0.0 to 1.0
		normalizedMagnitude = magnitude / (32767 - INPUT_DEADZONE);
	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		magnitude = 0.0;
		normalizedMagnitude = 0.0;
		normalizedRX = 0.0; // each controller has different initial setting, and seems not to be 0
		normalizedRY = 0.0; // since user doesn't push the bar, we just manually set it to 0.
	}
	controller_data[cNum].RThumb_magnitude = normalizedMagnitude;
	controller_data[cNum].RThumb_X_direction = normalizedRX;
	controller_data[cNum].RThumb_Y_direction = normalizedRY;
	if (normalizedMagnitude > 0) { // FOR debug use, only shows the status when we push the bar
		//std::cout << "right magnitude is:" << normalizedMagnitude << std::endl;
		//std::cout << "right bar direction in x axis is:" << normalizedRX << std::endl;
		//std::cout << "right bar direction in y axis is:" << normalizedRY << std::endl;
		//repeat for right thumb stick
	}
}
void checkButtons(XINPUT_STATE state, int cNum) {
	controller_data[cNum].LT = state.Gamepad.bLeftTrigger; // this trigger has a numbebr in range 0-255;
	controller_data[cNum].RT = state.Gamepad.bRightTrigger; // we can indicate how much force does the user trigger it

	int x = state.Gamepad.wButtons;

	//Y
	if (x >= button_array[13]) {
		controller_data[cNum].Y = true;
		x -= button_array[13];
	}
	else {
		controller_data[cNum].Y = false;
	}

	//X
	if (x >= button_array[12]) {
		controller_data[cNum].X = true;
		x -= button_array[12];
	}
	else {
		controller_data[cNum].X = false;
	}
	
	//B
	if (x >= button_array[11]) {
		controller_data[cNum].B = true;
		x -= button_array[11];
	}
	else {
		controller_data[cNum].B = false;
	}
	
	//A
	if (x >= button_array[10]) {
		controller_data[cNum].A = true;
		x -= button_array[10];
	}
	else {
		controller_data[cNum].A = false;
	}

	//RB
	if (x >= button_array[9]) {
		controller_data[cNum].RB = true;
		x -= button_array[9];
	}
	else {
		controller_data[cNum].RB = false;
	}

	//LB
	if (x >= button_array[8]) {
		controller_data[cNum].LB = true;
		x -= button_array[8];
	}
	else {
		controller_data[cNum].LB = false;
	}

	//R3
	if (x >= button_array[7]) {
		controller_data[cNum].R3 = true;
		x -= button_array[7];
	}
	else {
		controller_data[cNum].R3 = false;
	}

	//L3
	if (x >= button_array[6]) {
		controller_data[cNum].L3 = true;
		x -= button_array[6];
	}
	else {
		controller_data[cNum].L3 = false;
	}

	//BACK
	if (x >= button_array[5]) {
		controller_data[cNum].BACK = true;
		x -= button_array[5];
	}
	else {
		controller_data[cNum].BACK = false;
	}
	
	//START
	if (x >= button_array[4]) {
		controller_data[cNum].START = true;
		x -= button_array[4];
	}
	else {
		controller_data[cNum].START = false;
	}

	//RIGHT
	if (x >= button_array[3]) {
		controller_data[cNum].RIGHT = true;
		x -= button_array[3];
	}
	else {
		controller_data[cNum].RIGHT = false;
	}

	//LEFT
	if (x >= button_array[2]) {
		controller_data[cNum].LEFT = true;
		x -= button_array[2];
	}
	else {
		controller_data[cNum].LEFT = false;
	}

	//DOWN
	if (x >= button_array[1]) {
		controller_data[cNum].DOWN = true;
		x -= button_array[1];
	}
	else {
		controller_data[cNum].DOWN = false;
	}

	//UP
	if (x >= button_array[0]) { 
		controller_data[cNum].UP = true;
		x -= button_array[0];
	}
	else {
		controller_data[cNum].UP = false;
	}

}