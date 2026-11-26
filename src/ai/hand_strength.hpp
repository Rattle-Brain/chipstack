#pragma once

#include "core/cards/card.hpp"

namespace HandStrength {

float preflopStrength(int rank1, int rank2, bool suited);
float postflopStrength(const Cards& holeCards, const Cards& communityCards);
int countDrawOuts(const Cards& holeCards, const Cards& communityCards);

} // namespace HandStrength
