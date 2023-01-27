#pragma once
#include <Boilerplate/Window.h>
#define INPUT_DEADZONE 7849

ConStruct controller_data;


int button_array[14]= { 1,2,4,8,16,32,64,128,256,512,4096,8192,16384,32768 };
bool flag = true;
DWORD threadid;
HANDLE threadhandle;
XboxInput::XboxInput() {
	
}


void XboxInput::run()
{
	//std::thread controller_input_thread(update_controller_thread,NULL);
	threadhandle = CreateThread(0,0,update_controller_thread,0,0,&threadid);
}

void XboxInput::stop() {
	flag = false;
	WaitForSingleObject(threadhandle, INFINITE);
	CloseHandle(threadhandle);
}

void XboxInput::update() {
	data = controller_data;
}




DWORD WINAPI update_controller_thread(LPVOID lpParameter) {
	int time_before = 0;
	int time_after = 0;
	int time_diff = 0;
	while (flag) {
		//check if the controller is connected or not
		DWORD dwResult;

		//for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)  -----this will be useful when multi controller connected
		//{

		XINPUT_STATE state;
		//SecureZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(0, &state); //assume only one Xbox controller is connected

		time_diff = time_after - time_before;
		if (time_diff >= 5) {  // CHECK IF controller is connected every 5 seconds ?
			time_before = static_cast<int>( time(NULL) );

			if (dwResult == ERROR_SUCCESS)
			{
				std::cout << "Controller connected" << std::endl;
			}
			else
			{
				std::cout << "Warning! Warning! We lost him!" << std::endl;
			}
			//}

			time_after = static_cast<int>( time(NULL) );
		}
		else {
			time_after = static_cast<int>( time(NULL) );
		}

		if (dwResult == ERROR_SUCCESS) {
			checkLeftThumbBar(state);
			checkRightThumbBar(state);
			checkButtons(state);
		}
	}
	return 0;
}
void checkLeftThumbBar(XINPUT_STATE state) {
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
	controller_data.LThumb_magnitude = normalizedMagnitude;
	controller_data.LThumb_X_direction = normalizedLX;
	controller_data.LThumb_Y_direction = normalizedLY;
	if (normalizedMagnitude > 0) { // FOR debug use, only shows the status when we push the bar
		std::cout << "magnitude is:" << normalizedMagnitude << std::endl;
		std::cout << "left bar direction in x axis is:" << normalizedLX << std::endl;
		std::cout << "left bar direction in y axis is:" << normalizedLY << std::endl;
		//repeat for right thumb stick
	}

}
void checkRightThumbBar(XINPUT_STATE state) {
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
	controller_data.RThumb_magnitude = normalizedMagnitude;
	controller_data.RThumb_X_direction = normalizedRX;
	controller_data.RThumb_Y_direction = normalizedRY;
	if (normalizedMagnitude > 0) { // FOR debug use, only shows the status when we push the bar
		std::cout << "right magnitude is:" << normalizedMagnitude << std::endl;
		std::cout << "right bar direction in x axis is:" << normalizedRX << std::endl;
		std::cout << "right bar direction in y axis is:" << normalizedRY << std::endl;
		//repeat for right thumb stick
	}
}
void checkButtons(XINPUT_STATE state) {
	controller_data.LT = state.Gamepad.bLeftTrigger; // this trigger has a numbebr in range 0-255;
	controller_data.RT = state.Gamepad.bRightTrigger; // we can indicate how much force does the user trigger it

	int x = state.Gamepad.wButtons;

	//Y
	if (x >= button_array[13]) {
		controller_data.Y = true;
		x -= button_array[13];
	}
	else {
		controller_data.Y = false;
	}

	//X
	if (x >= button_array[12]) {
		controller_data.X = true;
		x -= button_array[12];
	}
	else {
		controller_data.X = false;
	}
	
	//B
	if (x >= button_array[11]) {
		controller_data.B = true;
		x -= button_array[11];
	}
	else {
		controller_data.B = false;
	}
	
	//A
	if (x >= button_array[10]) {
		controller_data.A = true;
		x -= button_array[10];
	}
	else {
		controller_data.A = false;
	}

	//RB
	if (x >= button_array[9]) {
		controller_data.RB = true;
		x -= button_array[9];
	}
	else {
		controller_data.RB = false;
	}

	//LB
	if (x >= button_array[8]) {
		controller_data.LB = true;
		x -= button_array[8];
	}
	else {
		controller_data.LB = false;
	}

	//R3
	if (x >= button_array[7]) {
		controller_data.R3 = true;
		x -= button_array[7];
	}
	else {
		controller_data.R3 = false;
	}

	//L3
	if (x >= button_array[6]) {
		controller_data.L3 = true;
		x -= button_array[6];
	}
	else {
		controller_data.L3 = false;
	}

	//BACK
	if (x >= button_array[5]) {
		controller_data.BACK = true;
		x -= button_array[5];
	}
	else {
		controller_data.BACK = false;
	}
	
	//START
	if (x >= button_array[4]) {
		controller_data.START = true;
		x -= button_array[4];
	}
	else {
		controller_data.START = false;
	}

	//RIGHT
	if (x >= button_array[3]) {
		controller_data.RIGHT = true;
		x -= button_array[3];
	}
	else {
		controller_data.RIGHT = false;
	}

	//LEFT
	if (x >= button_array[2]) {
		controller_data.LEFT = true;
		x -= button_array[2];
	}
	else {
		controller_data.LEFT = false;
	}

	//DOWN
	if (x >= button_array[1]) {
		controller_data.DOWN = true;
		x -= button_array[1];
	}
	else {
		controller_data.DOWN = false;
	}

	//UP
	if (x >= button_array[0]) { 
		controller_data.UP = true;
		x -= button_array[0];
	}
	else {
		controller_data.UP = false;
	}

}