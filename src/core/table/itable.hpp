#ifndef ITABLE_HPP
#define ITABLE_HPP

#include "core/common/crupier.hpp"
#include "core/common/game_observer.hpp"
#include <vector>
#include <functional>

class ITable {
  public:
    virtual ~ITable() {}

    void setObserver(IGameObserver* obs) { observer_ = obs; }

    void addPlayer(IPlayer* player) {
      players.push_back(player);
    }

    void removePlayer(int index) {
      players.erase(players.begin() + index);
    }

    int getPlayerCount() const {
      return players.size();
    }

    const std::vector<IPlayer*>& getPlayers() const {
      return players;
    }

    void removeBustedPlayers() {
      size_t i = 0;
      while (i < players.size()) {
        if (players[i]->isBustedOut()) {
          if (observer_) observer_->onPlayerBustedOut(players[i]->getName());
          players.erase(players.begin() + i);
        } else {
          i++;
        }
      }
    }

    std::function<void(IPlayer*)> preActionCallback;
    std::function<void(const std::string&)> postPhaseCallback;

    virtual void deal() = 0;
    virtual void collectCards() = 0;

  protected:
    std::vector<IPlayer*> players;
    Crupier crupier;
    IGameObserver* observer_ = nullptr;
};

#endif
