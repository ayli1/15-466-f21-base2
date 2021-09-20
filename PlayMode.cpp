#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("wok_it_gurl.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("wok_it_gurl.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	/*
	for (auto &transform : scene.transforms) {
		if (transform.name == "Hip.FL") hip = &transform;
		else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
		else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	}
	if (hip == nullptr) throw std::runtime_error("Hip not found.");
	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	hip_base_rotation = hip->rotation;
	upper_leg_base_rotation = upper_leg->rotation;
	lower_leg_base_rotation = lower_leg->rotation;
	*/

	//get pointers to thingz
	//Referenced: https://github.com/lassyla/game2/blob/master/FishMode.cpp
	for (auto &drawable : scene.drawables) {
		if (drawable.transform->name == "Wok Body") wok = (drawable.transform);
		else if (drawable.transform->name == "Leek") {
			leek = (drawable.transform);
			leek_vertex_type  = drawable.pipeline.type;
			leek_vertex_start = drawable.pipeline.start;
			leek_vertex_count = drawable.pipeline.count;
		}
		else if (drawable.transform->name == "Rock") {
			rock = (drawable.transform);
			rock_vertex_type  = drawable.pipeline.type;
			rock_vertex_start = drawable.pipeline.start;
			rock_vertex_count = drawable.pipeline.count;
		}
		else if (drawable.transform->name == "Heart") {
			heart = (drawable.transform);
			heart_vertex_type  = drawable.pipeline.type;
			heart_vertex_start = drawable.pipeline.start;
			heart_vertex_count = drawable.pipeline.count;
		}
	}
	if (wok   == nullptr) throw std::runtime_error("Wok not found.");
	if (leek  == nullptr) throw std::runtime_error("Leek not found.");
	if (rock  == nullptr) throw std::runtime_error("Rock not found.");
	if (heart == nullptr) throw std::runtime_error("Heart not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	/*
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);

	hip->rotation = hip_base_rotation * glm::angleAxis(
		glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
		glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	*/

	/*
	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}
	*/

	//move wok:
	{
		constexpr float WokSpeed = 2.0f;
		if (left.pressed  && !right.pressed) wok->position.x -= WokSpeed * elapsed;
		if (!left.pressed && right.pressed)  wok->position.x += WokSpeed * elapsed;
		if (down.pressed  && !up.pressed)    wok->position.y -= WokSpeed * elapsed;
		if (!down.pressed && up.pressed)     wok->position.y += WokSpeed * elapsed;

		if (wok->position.x > table_radius)  wok->position.x = table_radius;
		if (wok->position.x < -table_radius) wok->position.x = -table_radius;
		if (wok->position.y > table_radius)  wok->position.y = table_radius;
		if (wok->position.y < -table_radius) wok->position.y = -table_radius;

	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	//make it rain (i.e., new drop) every drop_period
	//Referenced: https://github.com/lassyla/game2/blob/master/FishMode.cpp
	time_passed += elapsed;
	if (time_passed > drop_period) {
		time_passed = 0.0f;

		Precipitation new_drop;
		new_drop.transform = new Scene::Transform;

		//Randomly choose the type of item
		int choose_item = rand() % 7;
		if (choose_item <= 3) { //leek
			*new_drop.transform = *leek; // So that the initial transform is same as arranged in scene; now all that needs to be changed is position
										 // DON'T do this if it has a parent... note the warning in Scene.hpp about copy-constructing transforms
			new_drop.delta_score = 1;
			new_drop.type  = leek_vertex_type;
			new_drop.start = leek_vertex_start;
			new_drop.count = leek_vertex_count;
		}
		else if ((choose_item == 4) || (choose_item == 5)) { //rock
			*new_drop.transform = *rock;
			new_drop.delta_score = -1;
			new_drop.type  = rock_vertex_type;
			new_drop.start = rock_vertex_start;
			new_drop.count = rock_vertex_count;
		}
		else if (choose_item == 6) { //heart
			*new_drop.transform = *heart;
			new_drop.delta_score = 5;
			new_drop.type  = heart_vertex_type;
			new_drop.start = heart_vertex_start;
			new_drop.count = heart_vertex_count;
		}

		int table_radius_hundredths = int(table_radius * 100.0f);
		new_drop.transform->position.x = 0.0f;//float(rand() % table_radius_hundredths) / 100.0f;
		new_drop.transform->position.y = 0.0f;//float(rand() % table_radius_hundredths) / 100.0f;
		new_drop.transform->position.z = 5.0f;


		//Add to drawables
		scene.drawables.emplace_back(new_drop.transform);

		Scene::Drawable& drawable = scene.drawables.back();
		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao   = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type  = new_drop.type;
		drawable.pipeline.start = new_drop.start;
		drawable.pipeline.count = new_drop.count;

		//Add to vector of all drops
		drops.push_back(new_drop);
	}

	for (size_t i = 0; i < drops.size(); i++) {
		Precipitation& drop = drops[i];

		//If it hits the table...
		if (drop.transform->position.z < 0.5f) {
			std::cout << "Hit table" << std::endl;
			std::cout << "Wok position: " << glm::to_string(wok->position) << std::endl;
			std::cout << "Drop position: " << glm::to_string(drop.transform->position) << std::endl;
			//... and it lands in the wok, change the score according to the item's value
			if (glm::distance(glm::vec2(wok->position.x, wok->position.y),
			    glm::vec2(drop.transform->position.x, drop.transform->position.y)) < wok_radius) {
				score += drop.delta_score;
			}
			//Get rid of this item!! We don't want them anymore >:(
			//First remove from drawables...
			//Referenced: https://stackoverflow.com/questions/16445358/stdfind-object-by-member
			std::list< Scene::Drawable >::iterator it;
			it = std::find_if(scene.drawables.begin(), scene.drawables.end(),
				[&](Scene::Drawable& d) { return d.transform == drop.transform; });
			if (it != scene.drawables.end()) {
				scene.drawables.erase(it);
			}
			else {
				std::cout << "Hey, couldn't find that drawable ??" << std::endl;
			}

			//... now remove from drops vector
			//Referenced: https://stackoverflow.com/questions/3385229/c-erase-vector-element-by-value-rather-than-by-position
			drops.erase(drops.begin() + i);
		}
		//Otherwise, if it's still mid-air, keep it goin'
		else {
			// Twirl downwards the way a heart vessel or stamina wheel in Breath of the Wild does <3
			//drop.transform->rotation *= glm::angleAxis(rotation_speed * elapsed, glm::vec3(0.0f, 0.0f, 1.0f));
			drop.transform->position.z -= drop_speed * elapsed;
		}
	}

}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves wok; escape ungrabs mouse; score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves wok; escape ungrabs mouse; score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
