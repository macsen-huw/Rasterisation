#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs(bool cursorOff, bool wasJustOn);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getCameraPosition();
#endif