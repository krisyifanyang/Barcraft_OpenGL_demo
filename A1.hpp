#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;





private:
	void initGrid();
	void drawcube(int xval, int yval, int cc);
	void drawIndicator();


	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	GLuint m_cube_vao; // Vertex Array Object
	GLuint m_cube_vbo; // Vertex Buffer Object

	GLuint m_indicator_vao; // Vertex Array Object
	GLuint m_indicator_vbo; // Vertex Buffer Object
	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[8][3];				// 8 colour with corresponding RGB
	int current_col;					// inticator of current colour
	int xValue;								// indicate current x in gird
	int yValue;								// indicate current y in gird
	int preX;									// indicate previous one step  x in gird
	int preY;									// indicate previous one step  y in gird
	int gridHeight[16][16];		// a grid for Height recording
	Grid *mygrid;							// a grid for colour recording, Grid is from grid.hpp
	float m_shape_rotation;		// track rotation angle
	float m_shape_size;				// track size for scaling
	bool click_left;					// track whether mouse left key is clicked or released
	bool click_right;					// track whether mouse right key is clicked or released
};
