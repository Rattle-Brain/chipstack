#ifndef BETTING_ROUND_HPP
#define BETTING_ROUND_HPP

#include "poker_action.hpp"
#include "betting_context.hpp"
#include "core/common/iplayer.hpp"
#include "core/common/game_observer.hpp"
#include <vector>
#include <functional>

struct PlayerSeat {
    IPlayer* player;
    PlayerHandState state;
    int currentBet;
    int totalInvested;
};

class BettingRound {
  public:
    BettingRound(std::vector<PlayerSeat>& seats, int pot, int bigBlind,
                 IGameObserver* observer = nullptr,
                 std::function<void(IPlayer*)> preActionCallback = nullptr);

    int execute(BettingRoundType roundType,
                const std::vector<Card>& communityCards,
                int dealerButton, int startIndex);

    int countActivePlayers() const;
    int countNonFoldedPlayers() const;

  private:
    std::vector<PlayerSeat>& seats;
    int currentBet;
    int pot;
    int bigBlind;
    IGameObserver* observer_;
    std::function<void(IPlayer*)> preActionCallback_;

    int nextActiveIndex(int from) const;
    PokerAction validateAction(PokerAction requested, const PlayerSeat& seat) const;
    void applyAction(PlayerSeat& seat, PokerAction action);
    Position computePosition(int seatIndex, int dealerButton) const;
    BettingContext buildContext(int seatIndex, BettingRoundType roundType,
                               const std::vector<Card>& communityCards,
                               int dealerButton, int playersToAct) const;
};

#endif
