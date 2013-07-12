#include "ifc_object.h"

using cppw::Instance;
using cppw::Application_instance;

namespace ifc_interface {

const Instance * ifc_object::as_instance() const {
	struct v : public boost::static_visitor<const Instance *> {
		const Instance * operator () (const Instance & inst) const {
			return &inst;
		}
		const Instance * operator () (const Application_instance & inst) const
		{
			return static_cast<const Instance *>(&inst);
		}
	};
	return boost::apply_visitor(v(), data_);
}

Application_instance * ifc_object::as_app_instance() {
	struct v : public boost::static_visitor<Application_instance *> {
		Application_instance * operator () (const Instance & /*inst*/) const {
			return __nullptr;
		}
		Application_instance * operator () (Application_instance & inst) const
		{
			return &inst;
		}
	};
	return boost::apply_visitor(v(), data_);
}

} // namespace ifc_adapter