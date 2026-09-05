#include "shared_ptr.h"

lab::shared_ptr<int> from_other_translation_unit() {
    return lab::make_shared<int>(106);
}
