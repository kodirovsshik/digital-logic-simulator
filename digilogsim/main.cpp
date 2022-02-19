
#include "engine.hpp"

#include <iostream>
#include <utility>

#include <ksn/metapr.hpp>



class main_app_t;

template<std::integral T>
class ttl_printing_layer :
	public layer_base<main_app_t>
{
	T value;

public:
	template<ksn::universal_reference<T> U>
	ttl_printing_layer(U val)
		: value(std::forward<T>(val))
	{
	}

	virtual bool update(app_t*) noexcept
	{
		std::cout << this->value << std::endl;
		return --this->value > 0;
	}
};

class main_app_t
{
	using my_t = main_app_t;
	using app_t = app_base_t<my_t>;

public:
	void init(app_t* app)
	{
		app->engine.insert_layer(std::make_unique<ttl_printing_layer<int>>(100));
		app->engine.insert_layer(std::make_unique<ttl_printing_layer<int>>(1));
		app->engine.insert_layer(std::make_unique<ttl_printing_layer<int>>(10));
	}
};


void digilog_main()
{
	app_base_t<main_app_t> app;
	app.start();
}
