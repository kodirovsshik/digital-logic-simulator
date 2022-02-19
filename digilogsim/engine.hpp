
#ifndef _DIGILOG_ENGINE_HPP_
#define _DIGILOG_ENGINE_HPP_


#include <ksn/window_gl.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/time.hpp>

#include <list>
#include <memory>
#include <bitset>

#include "errors.hpp"

#include <GL/glew.h>


template<class>
class app_base_t;

template<class app_data_t>
class layer_base
{
public:
	using app_t = app_base_t<app_data_t>;

	virtual bool update(app_t*) noexcept { return true; };
	virtual void render(app_t*) noexcept {};

	virtual ~layer_base() noexcept = 0 {};
};



template<class app_data_t>
class app_base_t
{
public:

	using resolution_t = ksn::vec<2, uint16_t>;
	using app_t = app_base_t<app_data_t>;
	using layer_t = layer_base<app_data_t>;


	app_base_t() noexcept {}
	app_base_t(const app_base_t&) = delete;
	app_base_t(app_base_t&&) = delete;

	app_base_t& operator=(const app_base_t&) = delete;
	app_base_t& operator=(app_base_t&&) = delete;



private:
	app_data_t app;

	class engine_t
	{
		friend class app_base_t;

	public:
		
		struct layer_attributes
		{
			bool exclusive_render : 1 = false;
			bool exclusive_update : 1 = false;
		};


	private:

		struct engine_private_data_t
		{
			std::list<std::unique_ptr<layer_t>> layers;
			std::list<layer_attributes> layers_attributes;
			ksn::stopwatch update_stopwatch;
		} private_data;


		void render(app_t*) noexcept;
		void poll() noexcept;
		void update(app_t*) noexcept;

		void init() noexcept;
		void app_start_notify() noexcept;


	public:

		struct engine_data_t
		{
			ksn::window_gl_t window;
			resolution_t window_resolution = { 640, 480 };
			float last_main_loop_iteration_time = 0;

			std::bitset<(int)ksn::keyboard_button_t::buttons_count> keyboard_pressed;
			std::bitset<(int)ksn::keyboard_button_t::buttons_count> keyboard_released;
			std::bitset<(int)ksn::keyboard_button_t::buttons_count> keyboard_down;

			std::bitset<(int)ksn::mouse_button_t::buttons_count> mouse_pressed;
			std::bitset<(int)ksn::mouse_button_t::buttons_count> mouse_released;
			std::bitset<(int)ksn::mouse_button_t::buttons_count> mouse_down;

			bool close_requested : 1 = false;
		} data;


		struct engine_config_t
		{
			bool reset_keyboard_status_on_lost_focus : 1 = true;
			bool reset_mouse_status_on_lost_focus : 1 = true;
			//bool use_async_displaying : 1 = true;
		} config;

		void set_resolution(resolution_t) noexcept;

		void insert_layer(std::unique_ptr<layer_t>&&, layer_attributes = {});
	};


	void main_loop();

public:

	engine_t engine;

	void start();
};







template<class app_data_t>
void app_base_t<app_data_t>::main_loop()
{
	while (true)
	{
		this->engine.render(this);
		this->engine.poll();
		this->engine.update(this);
	}
}

template<class app_data_t>
void app_base_t<app_data_t>::start()
{
	this->engine.init();
	this->app.init(this);
	this->engine.app_start_notify();
	this->main_loop();
}




template<class app_data_t>
void app_base_t<app_data_t>::engine_t::init() noexcept
{
	this->set_resolution(this->data.window_resolution);
	this->data.window.set_framerate(60);
}

template<class app_data_t>
void app_base_t<app_data_t>::engine_t::render(app_t* app) noexcept
{
	auto p_layer = this->private_data.layers.rbegin();
	auto p_laper_attribs = this->private_data.layers_attributes.rbegin();

	for (; p_layer != this->private_data.layers.rend();
		++p_layer, ++p_laper_attribs)
	{
		(*p_layer)->render(app);
		if (p_laper_attribs->exclusive_render)
			break;
	}

	this->data.window.tick();
}

template<class app_data_t>
void app_base_t<app_data_t>::engine_t::poll() noexcept
{
	this->data.keyboard_pressed.reset();
	this->data.keyboard_released.reset();
	this->data.mouse_pressed.reset();
	this->data.mouse_released.reset();

	ksn::event_t ev;
	while (this->data.window.poll_event(ev))
	{
		switch (ev.type)
		{
		case ksn::event_type_t::keyboard_press:
			this->data.keyboard_pressed.set((size_t)ev.keyboard_button_data.button);
			this->data.keyboard_down.set((size_t)ev.keyboard_button_data.button);
			break;

		case ksn::event_type_t::keyboard_release:
			this->data.keyboard_released.set((size_t)ev.keyboard_button_data.button);
			this->data.keyboard_down.reset((size_t)ev.keyboard_button_data.button);
			break;

		case ksn::event_type_t::mouse_press:
			this->data.mouse_pressed.set((size_t)ev.mouse_button_data.button);
			this->data.mouse_down.set((size_t)ev.mouse_button_data.button);
			break;

		case ksn::event_type_t::mouse_release:
			this->data.mouse_released.set((size_t)ev.mouse_button_data.button);
			this->data.mouse_down.reset((size_t)ev.mouse_button_data.button);
			break;

		case ksn::event_type_t::focus_lost:
			if (this->config.reset_keyboard_status_on_lost_focus)
			{
				this->data.keyboard_released |= this->data.keyboard_down;
				this->data.keyboard_down.reset();
			}
			if (this->config.reset_mouse_status_on_lost_focus)
			{
				this->data.mouse_released |= this->data.mouse_down;
				this->data.mouse_down.reset();
			}
			break;

		case ksn::event_type_t::close:
			this->data.close_requested = true;
			break;
		}
	}
}

template<class container_t, class iterator_t>
static void erase_and_advance(container_t& cont, std::reverse_iterator<iterator_t>& iter)
{
	auto temp = iter;
	++temp;
	cont.erase(temp.base());
}

template<class app_data_t>
void app_base_t<app_data_t>::engine_t::update(app_t* app) noexcept
{
	auto dt = this->private_data.update_stopwatch.restart();
	if (dt.as_nsec() == 0)
	{
		auto fps = this->data.window.get_framerate();
		if (fps != 0)
			dt = ksn::time((int64_t)(1e9f / fps));
	}

	this->data.last_main_loop_iteration_time = dt.as_float_sec();

	auto p_layer = this->private_data.layers.rbegin();
	auto p_layer_attribs = this->private_data.layers_attributes.rbegin();

	while (p_layer != this->private_data.layers.rend())
	{
		bool keep = (*p_layer)->update(app);
		bool exclusive = p_layer_attribs->exclusive_update;

		if (!keep)
		{
			erase_and_advance(this->private_data.layers, p_layer);
			erase_and_advance(this->private_data.layers_attributes, p_layer_attribs);
		}
		else
		{
			++p_layer;
			++p_layer_attribs;
		}

		if (exclusive)
			break;
	}
}

template<class app_data_t>
void app_base_t<app_data_t>::engine_t::app_start_notify() noexcept
{
	this->data.window.show();
	this->private_data.update_stopwatch.start();
	this->data.window.tick();
}

template<class app_data_t>
void app_base_t<app_data_t>::engine_t::set_resolution(resolution_t res) noexcept
{
	if (this->data.window.is_open())
	{
		this->data.window.set_client_size(res);
	}
	else
	{
		ksn::window_style_t style = 0;
		style |= ksn::window_style::border;
		style |= ksn::window_style::caption;
		style |= ksn::window_style::close_button;
		style |= ksn::window_style::hidden;

		ksn::window_gl_t::context_settings context{};
		context.ogl_debug = true;

		uint8_t ogl_versions[][2] =
		{
			4, 6,
			4, 3,
			3, 1,
			2, 0,
		};

		int open_result = -1;
		for (const auto& [v_maj, v_min] : ogl_versions)
		{
			context.ogl_version_major = v_maj;
			context.ogl_version_minor = v_min;
			open_result = this->data.window.open(res[0], res[1], "", context, style);
			if (open_result == ksn::window_open_result::ok)
				break;
		}
		if (open_result != ksn::window_open_result::ok)
			excp_raise("Failed to open the window", "Error code " + std::to_string(open_result));

		this->data.window.context_make_current();

		printf("%s\n", glGetString(GL_VERSION));
		printf("%s\n", glGetString(GL_VENDOR));
		printf("%s\n", glGetString(GL_RENDERER));
	}
}

template<class app_data_t>
inline void app_base_t<app_data_t>::engine_t::insert_layer(std::unique_ptr<layer_t>&& ptr, layer_attributes attributes)
{
	this->private_data.layers.push_back(std::move(ptr));
	this->private_data.layers_attributes.push_back(attributes);
}


#endif //!_DIGILOG_ENGINE_HPP_
