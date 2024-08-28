#ifndef TNFR_H_
#define TNFR_H_

#include <string>
#include <vector>

#include "adv.h"

namespace tnfr
{
	bool LoadScenario(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData);
}
#endif // !TNFR_H_
