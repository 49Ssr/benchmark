#pragma once
#include <cstdint>

enum class ValidationStatus {	//create set of named states. eg. ValidationStatus::Passed;
	NotChecked,
	Passed,
	Failed
};

struct ValidationResult {	//groups 2 related outputs: status and 64-bit fingerprint
	ValidationStatus status = ValidationStatus::NotChecked;
	std::uint64_t fingerprint = 0;
};