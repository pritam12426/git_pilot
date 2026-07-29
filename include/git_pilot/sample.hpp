#pragma once

#include <string>

namespace git_pilot {

class Sample
{
public:
	explicit Sample(std::string name);

	void process();

private:
	std::string name_;
};

}  // namespace git_pilot
