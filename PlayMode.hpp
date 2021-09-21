#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	struct Precipitation {
		Scene::Transform* transform;
		int delta_score = 0; // +1 for leek; -1 for rock; +5 for heart
		GLenum type     = GL_TRIANGLES; //what sort of primitive to draw; passed to glDrawArrays
		GLuint start    = 0; //first vertex to draw; passed to glDrawArrays
		GLuint count    = 0; //number of vertices to draw; passed to glDrawArrays
	};

	//thingz:
	Scene::Transform *wok   = nullptr;

	Scene::Transform *leek   = nullptr;
	GLenum leek_vertex_type  = GL_TRIANGLES; //what sort of primitive to draw; passed to glDrawArrays
	GLuint leek_vertex_start = 0; //first vertex to draw; passed to glDrawArrays
	GLuint leek_vertex_count = 0; //number of vertices to draw; passed to glDrawArrays

	Scene::Transform *rock   = nullptr;
	GLenum rock_vertex_type  = GL_TRIANGLES; //what sort of primitive to draw; passed to glDrawArrays
	GLuint rock_vertex_start = 0; //first vertex to draw; passed to glDrawArrays
	GLuint rock_vertex_count = 0; //number of vertices to draw; passed to glDrawArrays

	Scene::Transform *heart   = nullptr;
	GLenum heart_vertex_type  = GL_TRIANGLES; //what sort of primitive to draw; passed to glDrawArrays
	GLuint heart_vertex_start = 0; //first vertex to draw; passed to glDrawArrays
	GLuint heart_vertex_count = 0; //number of vertices to draw; passed to glDrawArrays

	std::vector<Precipitation> drops; //Vector of all current falling items

	float table_radius   = 3.5f;
	float wok_radius     = 1.0f;
	float wok_speed      = 4.0f;
	float drop_period    = 3.0f; // Amount of time until next object drops
	float time_passed    = 3.0f;
	float drop_speed     = 4.0f; // How fast the items fall (no gravitational acceleration, sry)
	float rotation_speed = 1.2f; // How fast the items gracefully twirl downward
	
	int score = 0;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
