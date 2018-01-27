#ifndef MATHDEFS_HPP
#define MATHDEFS_HPP

#define GLM_FORCE_LEFT_HANDED

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#define PITCH 0
#define YAW 1
#define ROLL 2

// GLM's naming conventions really bother me
typedef glm::vec2 Vector2f;
typedef glm::vec3 Vector3f;
typedef glm::vec4 Vector4f;
typedef glm::mat3 Matrix3f;
typedef glm::mat4 Matrix4f;
typedef glm::quat Quaternion;

#endif // MATHDEFS_HPP