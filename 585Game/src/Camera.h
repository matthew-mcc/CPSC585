#pragma once
#include <glm.hpp>
#include <Entity.h>

using namespace std;
using namespace glm;

class Camera {

public:
	Camera();
	Camera(Entity* p, int pn);
	void updateCamera(float dt, shared_ptr<CallbackInterface> cptr, int numPlayers);

	mat4 view = mat4(1.0f);
	mat4 projection = mat4(1.0f);

	vec3 camera_previous_position = vec3(0.0f, 8.0f, -270.0f);
	float camera_target_forward = 0.0f;
	float camera_target_up = 1.5f;
	float camera_target_right = 0.0f;

private:
	void resetValue(float& target, float range, float desireValue, float speed, float step);
	void updateRadius(float base, float zoom);

	Entity* player;
	int playerNo;

	vec3 player_forward;
	vec3 player_right;
	vec3 player_up;
	vec3 player_pos;

	float camera_zoom_forward;
	float camera_zoom_up;

	vec3 eye_offset;
	vec3 target_offset;

	vec3 ResetVec;
	vec3 Camera_collision;
	vec3 Reset_collision;

	vec3 camera_target_position;
	vec3 camera_track_vector;

	float camera_position_forward = -7.5f;
	float camera_position_up = 3.5f;
	float camera_position_right = 0.0f;
	vec3 world_up = vec3(0.0f, 1.0f, 0.0f);
	float camera_radius = 7.5f;
	float rad_base = 7.5f;

	float camera_lag = 5.0f;
	float fov_rest = 45.f;
	float fov_boost = 57.f;
	float fov_change_speed = 400.0f;
	float fov = fov_rest;
};