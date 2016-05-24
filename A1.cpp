#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ),
	m_shape_size(1.0f),
	m_shape_rotation(0.0f)
{

	colour[0][0] = 0.78f;
	colour[0][1] = 0.64f;
	colour[0][2] = 0.8f;

	colour[1][0] = 0.3f;
	colour[1][1] = 0.3f;
	colour[1][2] = 0.8f;

	colour[2][0] = 0.45f;
	colour[2][1] = 0.6f;
	colour[2][2] = 0.12f;

	colour[3][0] = 0.1f;
	colour[3][1] = 0.9f;
	colour[3][2] = 0.5f;

	colour[4][0] = 0.23f;
	colour[4][1] = 0.76f;
	colour[4][2] = 0.91f;

	colour[5][0] = 0.8f;
	colour[5][1] = 0.4f;
	colour[5][2] = 0.5f;

	colour[6][0] = 0.57f;
	colour[6][1] = 0.23f;
	colour[6][2] = 0.1f;

	colour[7][0] = 0.3f;
	colour[7][1] = 0.05f;
	colour[7][2] = 0.6f;

	mygrid = new Grid (DIM);
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{

	xValue=0;
	yValue=0;
	preX=0;
	preY=0;
	click_left = false;
	click_right = false;

	// init gridHeight array to record grid height during operation
	for (int i=0; i<DIM; i++)
		for (int j=0;j<DIM;j++)
			gridHeight[i][j]=0;

	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );
	initGrid();


	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt(
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );
	proj = glm::perspective(
		glm::radians(   45.0f),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my*
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}



//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}


//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);



		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset" ) ) {

			// reset size and rotation
			m_shape_size = 1.0f;
			m_shape_rotation = 0.0f;
			// reset grid
			for (int i=0; i<DIM; i++)
				for (int j=0;j<DIM;j++)
					gridHeight[i][j]=0;
			//reset indicator
			xValue=0;
			yValue=0;
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour[0] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
			 mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();


		ImGui::PushID( 1 );
		ImGui::ColorEdit3( "##Colour", colour[1] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 1 ) ) {
			mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();


		ImGui::PushID( 2 );
		ImGui::ColorEdit3( "##Colour", colour[2] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 2 ) ) {
			mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();


		ImGui::PushID( 3 );
		ImGui::ColorEdit3( "##Colour", colour[3] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 3 ) ) {
			mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();


		ImGui::PushID( 4 );
		ImGui::ColorEdit3( "##Colour", colour[4] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 4 ) ) {
			mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();

		ImGui::PushID( 5 );
		ImGui::ColorEdit3( "##Colour", colour[5] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 5 ) ) {
			mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();


		ImGui::PushID( 6 );
		ImGui::ColorEdit3( "##Colour", colour[6] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 6 ) ) {
			mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();


		ImGui::PushID( 7 );
		ImGui::ColorEdit3( "##Colour", colour[7] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 7 ) ) {
			mygrid->setColour(xValue,yValue,current_col);
		}
		ImGui::PopID();

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	vec3 y_axis(0.0f,1.0f,0.0f);
	vec3 x_half(0.5f,0.0f,0.0f);

	// rotation and scaling display for each frame
  W*= glm::rotate(mat4(), m_shape_rotation, y_axis);
	W*= glm::scale(mat4(), vec3(m_shape_size));
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
	//	glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

    // now handle cube draw all cubes in grid every frame
    for(int i=0; i< DIM; i++)
			  for(int j=0; j< DIM; j++)
					if(gridHeight[i][j]!=0)
						drawcube(i,j,mygrid->getColour(i,j));

		// now draw indicator
		drawIndicator();
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}





void A1::drawIndicator(){
	float indicator_buffer_data[] = {
		xValue,			gridHeight[xValue][yValue],		yValue,
		xValue+1,		gridHeight[xValue][yValue],		yValue,
		xValue,			gridHeight[xValue][yValue],		yValue+1,
		xValue,			gridHeight[xValue][yValue],		yValue,
		xValue,			gridHeight[xValue][yValue],		yValue+1,
		xValue+1,		gridHeight[xValue][yValue],		yValue+1,
		xValue,			gridHeight[xValue][yValue],		yValue,
		xValue+1,		gridHeight[xValue][yValue],		yValue,
		xValue+1,		gridHeight[xValue][yValue],		yValue+1,
		xValue+1,		gridHeight[xValue][yValue],		yValue,
		xValue+1,		gridHeight[xValue][yValue],		yValue+1,
		xValue,			gridHeight[xValue][yValue],		yValue+1
	};
	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_indicator_vao );
	glBindVertexArray(m_indicator_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_indicator_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_indicator_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(indicator_buffer_data),
		indicator_buffer_data, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	//GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	glBindVertexArray( m_indicator_vao );
	glUniform3f( col_uni, 0.5f, 0.5f, 0.5f );
	glDrawArrays( GL_TRIANGLES, 0, 12 );
	CHECK_GL_ERRORS;


}

void A1::drawcube(int xval, int yval, int cc){

	 float g_vertex_buffer_data[] = {
		xval,			0.0f,												yval,
		xval,			0.0f, 											yval+1,
		xval, 		gridHeight[xval][yval], 		yval+1,
		xval+1, 	gridHeight[xval][yval],			yval,
		xval,			0.0f,												yval,
		xval,			gridHeight[xval][yval],			yval,
		xval+1,		0.0f, 											yval+1,
		xval,			0.0f,												yval,
		xval+1,		0.0f,												yval,
		xval+1, 	gridHeight[xval][yval],			yval,
		xval+1,		0.0f,												yval,
		xval,			0.0f,												yval,
		xval,			0.0f,												yval,
		xval, 		gridHeight[xval][yval],			yval+1,
		xval, 		gridHeight[xval][yval],			yval,
		xval+1,		0.0f, 											yval+1,
		xval,			0.0f,												yval+1,
		xval,			0.0f,												yval,
		xval, 		gridHeight[xval][yval], 		yval+1,
		xval,			0.0f,												yval+1,
		xval+1,		0.0f, 											yval+1,
		xval+1, 	gridHeight[xval][yval],			yval+1,
		xval+1,		0.0f,												yval,
		xval+1, 	gridHeight[xval][yval],			yval,
		xval+1,		0.0f,												yval,
		xval+1, 	gridHeight[xval][yval], 		yval+1,
		xval+1,		0.0f, 											yval+1,
		xval+1, 	gridHeight[xval][yval], 		yval+1,
		xval+1, 	gridHeight[xval][yval],			yval,
		xval, 		gridHeight[xval][yval],			yval,
		xval+1, 	gridHeight[xval][yval], 		yval+1,
		xval, 		gridHeight[xval][yval],			yval,
		xval, 		gridHeight[xval][yval], 		yval+1,
		xval+1, 	gridHeight[xval][yval], 		yval+1,
		xval, 		gridHeight[xval][yval],			yval+1,
		xval+1,		0.0f, 											yval+1
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
		g_vertex_buffer_data, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	//GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	glBindVertexArray( m_cube_vao );
	glUniform3f( col_uni, colour[cc][0], colour[cc][1], colour[cc][2] );
	glDrawArrays( GL_TRIANGLES, 0, 36 );
	CHECK_GL_ERRORS;

}




//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (int entered)
{
	bool eventHandled(false);

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A1::mouseMoveEvent(double xPos, double yPos)
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if(click_left)
			m_shape_rotation = m_shape_rotation + 0.01f;
		if(click_right)
			m_shape_rotation = m_shape_rotation - 0.01f;
	}
	eventHandled =true;
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		//  check left right mouse button is pressed or released
			if (actions== GLFW_PRESS && button==GLFW_MOUSE_BUTTON_LEFT )
				click_left=true;
			if(actions==GLFW_RELEASE && button==GLFW_MOUSE_BUTTON_LEFT)
				click_left=false;
			if (actions== GLFW_PRESS && button==GLFW_MOUSE_BUTTON_RIGHT )
				click_right=true;
			if(actions==GLFW_RELEASE && button==GLFW_MOUSE_BUTTON_RIGHT)
				click_right=false;
	}
	eventHandled =true;
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	if (yOffSet>0 && m_shape_size<1.8)
		m_shape_size+=0.3;
	if(yOffSet<0 && m_shape_size >0.2)
		m_shape_size-=0.3;

	eventHandled=true;
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	if( action == GLFW_PRESS  ) {





		if (key == GLFW_KEY_SPACE) {
// handle colour and height
			mygrid->setColour(xValue,yValue,current_col);
//			cout << "space key pressed" << endl;
			gridHeight[xValue][yValue]++;
			eventHandled = true;
		}

		if (key == GLFW_KEY_BACKSPACE) {
			//cout << "bkspace key pressed" << endl;
			if (gridHeight[xValue][yValue]>0) gridHeight[xValue][yValue]--;
			eventHandled = true;
		}

		if (key == GLFW_KEY_UP) {
			//update x and y and prex prey
			if (yValue>0){
				preX=xValue;
				preY=yValue;
				yValue--;
			}
			//if copy handle colour and height
			if (mods == GLFW_MOD_SHIFT)
					gridHeight[xValue][yValue]=gridHeight[preX][preY];
					mygrid->setColour(xValue,yValue,mygrid->getColour(preX,preY));
			//cout << "up" << endl;
			eventHandled = true;
		}

		if (key == GLFW_KEY_DOWN) {
		//	cout << "down" << endl;
			if (yValue<15){
				preX=xValue;
				preY=yValue;
				yValue++;
			}
			if (mods == GLFW_MOD_SHIFT)
					gridHeight[xValue][yValue]=gridHeight[preX][preY];
					mygrid->setColour(xValue,yValue,mygrid->getColour(preX,preY));
			eventHandled = true;
		}

		if (key == GLFW_KEY_LEFT) {
			//cout << "left" << endl;
			if (xValue>0){
				preX=xValue;
				preY=yValue;
				xValue--;
			}
			if (mods == GLFW_MOD_SHIFT)
					gridHeight[xValue][yValue]=gridHeight[preX][preY];
					mygrid->setColour(xValue,yValue,mygrid->getColour(preX,preY));
			eventHandled = true;
		}

		if (key == GLFW_KEY_RIGHT) {
			//cout << "right" << endl;
			if (xValue<15){
				preX=xValue;
				preY=yValue;
			  xValue++;
		  }
			if (mods == GLFW_MOD_SHIFT)
					gridHeight[xValue][yValue]=gridHeight[preX][preY];
					mygrid->setColour(xValue,yValue,mygrid->getColour(preX,preY));
			eventHandled = true;
		}

// quit program
		if (key == GLFW_KEY_Q){
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

//reset
		if (key == GLFW_KEY_R){
			// reset size and rotation
			m_shape_size = 1.0f;
			m_shape_rotation = 0.0f;
			// reset grid
			for (int i=0; i<DIM; i++)
				for (int j=0;j<DIM;j++)
					gridHeight[i][j]=0;
			//reset color picker
			xValue=0;
			yValue=0;
		}
	}

	return eventHandled;

}
