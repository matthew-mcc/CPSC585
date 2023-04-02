#include "Camera.h"
#include "PhysicsSystem.h"

using namespace std;

Camera::Camera() {}

Camera::Camera(Entity* p) {
	player = p;
}

void Camera::updateCamera(float dt, shared_ptr<CallbackInterface> cptr, int numPlayers) {
	player_forward = player->transform->getForwardVector();
	player_right = player->transform->getRightVector();
	player_up = player->transform->getUpVector();
	player_pos = player->transform->getPosition();

	camera_zoom_forward = clamp(1.0f + (float)player->nbChildEntities * 0.5f, 1.0f, 11.0f);
	camera_zoom_up = clamp(1.0f + (float)player->nbChildEntities * 0.4f, 1.0f, 11.0f);

	eye_offset = (camera_position_forward * player_forward * camera_zoom_forward) + (camera_position_right * player_right) + (camera_position_up * player_up * camera_zoom_up);
	target_offset = player_pos + (camera_target_forward * player_forward) + (camera_target_right * player_right) + (camera_target_up * player_up);

	ResetVec = player->transform->getRotation() * vec3(0.f, 3.5f * camera_zoom_up, -7.5f * camera_zoom_forward);
	Camera_collision = vec3(0.f);
	Reset_collision = vec3(1.f);

	camera_target_position = player_pos + eye_offset;
	camera_target_position.y = player_pos.y + camera_position_up + (float)player->nbChildEntities * 0.4f;
	camera_track_vector = camera_target_position - camera_previous_position;
	camera_track_vector = camera_track_vector * camera_lag * dt;
	camera_previous_position = vec3(translate(mat4(1.0f), camera_track_vector) * vec4(camera_previous_position, 1.0f));

	if (cptr->moveCamera) {
		vec3 lookOffset = camera_previous_position - player_pos;
		lookOffset = vec4(lookOffset, 0.f) * glm::rotate(glm::mat4(1.f), cptr->xAngle - 3.1415126f, world_up);
		lookOffset += player_pos;
		Camera_collision = PhysicsSystem::CameraRaycasting(camera_previous_position, camera_radius, 1.f);
		view = lookAt(lookOffset, target_offset, world_up);
	}
	else {
		Camera_collision = PhysicsSystem::CameraRaycasting(camera_previous_position, camera_radius, 1.f);
		Reset_collision = PhysicsSystem::CameraRaycasting(player_pos + ResetVec, camera_radius, 1.f);
		view = lookAt(camera_previous_position, target_offset, world_up);
	}

	if (Camera_collision.x != 0.f && Camera_collision.y != 0 && Camera_collision.z != 0) {//not sure which one to use XD
		camera_position_forward += Camera_collision.z;//* (float)timer->getDeltaTime();//camera_previous_position.z += 1.f; //* (float)timer->getDeltaTime();//cout << "???" << endl;
		camera_position_up = clamp(camera_position_up + Camera_collision.y, 3.5f, 100.f);
		camera_position_right -= Camera_collision.x;
		rad_base = clamp(rad_base - sqrtf(Camera_collision.z * Camera_collision.z + Camera_collision.x * Camera_collision.x), 0.5f, 7.5f);
		//timeTorset = 0.f;
	}
	else {
		if (Reset_collision.x == 0.f && Reset_collision.y == 0 && Reset_collision.z == 0 && !PhysicsSystem::CameraIntercetionRaycasting(player_pos + ResetVec)) { //lag to reset the camera
			float reset_speed = 50.f; // the speed to rset the camera
			float step_time = dt;
			float step_range = step_time * reset_speed;
			resetValue(camera_position_forward, step_range, -7.5f, reset_speed, step_time);
			resetValue(camera_position_up, step_range, 3.5f, reset_speed, step_time);
			resetValue(camera_position_right, step_range, 0.f, reset_speed, step_time);
			resetValue(rad_base, step_range, 7.5f, reset_speed, step_time);
		}
		//timeTorset += (float)timer->getDeltaTime();
	}
	updateRadius(rad_base, camera_zoom_forward);
	if (player->playerProperties->boost != 0) {
		if (fov < fov_boost) {
			fov = fov + ((fov_boost - fov) / fov_boost) * fov_change_speed * dt;
		}
	}
	else {
		if (fov > fov_rest) {
			fov = fov - ((fov - fov_rest) / fov) * (fov_change_speed / 2.f) * dt;
		}
	}
	if (numPlayers == 2) projection = perspective(radians(fov), (float)cptr->xres / ((float)cptr->yres / 2.f), 0.1f, 10000.0f);
	else projection = perspective(radians(fov), (float)cptr->xres / (float)cptr->yres, 0.1f, 10000.0f);
}

void Camera::resetValue(float& target, float range, float desireValue, float speed, float step) {
	if (target > desireValue)
		target -= speed * step;
	else if (target < desireValue)
		target += speed * step;

	if (target > (desireValue - range) && target < (desireValue + range))
		target = desireValue;
}

void Camera::updateRadius(float base, float zoom) {
	camera_radius = base + zoom;
}
