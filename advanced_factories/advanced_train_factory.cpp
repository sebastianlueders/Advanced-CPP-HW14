#include "advanced_factory.h"
#include "advanced_trains.h"
#include <string>
#include <iostream>
#include <memory>
using namespace std;
using namespace mpcs;

// using statements replace typedefs (and more)
using train_factory = abstract_factory<locomotive, freight_car, caboose>;
using model_train_factory
  = concrete_factory<train_factory, 
	                 model_locomotive, model_freight_car, model_caboose>;

// Compare how the using statement for model_train_factory is clearer
// than the equivalent typedef for real_train_factory
typedef
  concrete_factory
    <train_factory, real_locomotive, real_freight_car, real_caboose> 
  real_train_factory;

using paremeterized_model_train_factory
  =  parameterized_factory<train_factory, model>;

using parameterized_real_train_factory
  = parameterized_factory<train_factory, real>;

using flexible_train_factory 
	= abstract_factory 
		<locomotive(Engine), freight_car(long), caboose>;
using flexible_real_train_factory
  = concrete_factory
        <flexible_train_factory, real_locomotive, real_freight_car, real_caboose>;
using flexible_model_train_factory
= concrete_factory
<flexible_train_factory, model_locomotive, model_freight_car, model_caboose>;

int main()
{
  unique_ptr<train_factory> factory(make_unique<model_train_factory>());
  auto l(factory->create<locomotive>());
  l->blow_horn();
  factory = make_unique<parameterized_real_train_factory>();
  l = factory->create<locomotive>();
  l->blow_horn();
  unique_ptr<flexible_train_factory> 
    flexible_factory(make_unique<flexible_model_train_factory>());
  l = flexible_factory->create<locomotive>(Engine(800.0));

  l->blow_horn();
  flexible_factory->create<caboose>()->wave();

  factory = make_unique<parameterized_real_train_factory>();
  l = factory->create<locomotive>();
  l->blow_horn();

  return 0;
}
