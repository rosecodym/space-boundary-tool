#include "IfcModel.h"

#include "string_conversion.h"

namespace IfcInterface {
	
namespace {

const int MAX_PATH = 260;

} // namespace

IfcModel::IfcModel(String ^ path) 
	: database_(EdmDatabase::Instance()),
	  repo_(__nullptr),
	  model_(__nullptr)
{
	char buf[MAX_PATH];
	managed_string_to_native(buf, path, MAX_PATH);
	cppw::Step_reader(buf, "repo", buf).read();
	repo_ = database_->GetRepository("repo");
	model_ = new cppw::Open_model(repo_->get_model(buf), cppw::RW_access);
}

} // namespace IfcInterface