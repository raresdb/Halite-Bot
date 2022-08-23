#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <queue>
#include <map>
#include <iomanip>

#include "hlt.hpp"
#include "networking.hpp"

int main() {
    srand(time(NULL));

    std::cout.sync_with_stdio(0);

    unsigned char myID;
    hlt::GameMap presentMap; 

    getInit(myID, presentMap);
    sendInit("AmVenitPtPizza");

    class compareByScore
    {
    public:
        bool operator()(std::pair<hlt::Location, int> p1, std::pair<hlt::Location, int> p2) {
            return p1.second < p2.second;
        }
    };
    
    std::set<hlt::Move> moves;

    //priority queue for visiting all my sites starting from the borders
    std::priority_queue<std::pair<hlt::Location, int>, std::vector<std::pair<hlt::Location, int>>,
        compareByScore> visited;

    std::map<hlt::Location, int> scoresByLocation;

    while(true) {
        moves.clear();
        scoresByLocation.clear();

        // list of positions owned by ME
        std::list<hlt::Location> me;

        hlt::Location loc;
        for(unsigned short a = 0; a < presentMap.height; a++) {
            for(unsigned short b = 0; b < presentMap.width; b++) {
                if (presentMap.getSite({ b, a }).owner == myID) {
                    loc.x = b;
                    loc.y = a;
                    me.push_front(loc);
                }
            }
        }

        getFrame(presentMap);

        std::pair<hlt::Location, int> queue_entry;

        //introduce borders in the search queue
        for(auto itr = me.begin(); itr != me.end(); itr++) {
            for(auto dir : CARDINALS) {
                hlt::Location loc = presentMap.getLocation(*itr, dir);
                hlt::Site site = presentMap.getSite(*itr, dir);
                std::pair<hlt::Location, int> queue_entry1;

                if(site.owner != myID && scoresByLocation.find(loc) == scoresByLocation.end()) {
                    queue_entry1.first = loc;
                    queue_entry1.second = site.production* 5 - (site.strength * 7 / 10);
                    visited.push(queue_entry1);
                    scoresByLocation.insert(queue_entry1);
                }
            }
        }

        //introduce all squares in the queue
        while(!visited.empty()) {
            queue_entry = visited.top();
            visited.pop();

           for(auto dir : CARDINALS) {
                hlt::Location loc = presentMap.getLocation(queue_entry.first, dir);
                hlt::Site *site = &presentMap.getSite(queue_entry.first, dir);
                std::pair<hlt::Location, int> queue_entry1;

                if((*site).owner == myID && scoresByLocation.find(loc) == scoresByLocation.end()) {
                    queue_entry1.first = loc;
                    queue_entry1.second = queue_entry.second - (*site).production;
                    visited.push(queue_entry1);
                    (*site).direction = presentMap.reverse_dir(dir);
                    scoresByLocation.insert(queue_entry1);
                }
           }
        }

        //plant moves
        for(auto my_loc : me) {
            std::priority_queue<std::pair<hlt::Location, int>, std::vector<std::pair<hlt::Location, int>>,
                compareByScore> neutral_neighs;
            std::map<hlt::Location, int> neighDirByLoc;
            hlt::Site neigh_site;
            hlt::Location neigh_loc;
            hlt::Site my_site = presentMap.getSite(my_loc, STILL);
            hlt::Move move;
            
            for(auto dir : CARDINALS) {
                neigh_site = presentMap.getSite(my_loc, dir);
                neigh_loc = presentMap.getLocation(my_loc, dir);
                queue_entry.first = neigh_loc;
                queue_entry.second = neigh_site.production;//scoresByLocation.at(neigh_loc);

                if(neigh_site.owner != myID)
                    neutral_neighs.push(queue_entry);

                queue_entry.second = dir;
                neighDirByLoc.insert(queue_entry);
            }

            int found_neigh = 0;
            int border = 0;
            while(!neutral_neighs.empty()) {
                border++;
                queue_entry = neutral_neighs.top();
                neutral_neighs.pop();
                neigh_site = presentMap.getSite(queue_entry.first, STILL);

                if(neigh_site.strength >= my_site.strength) {
                    continue;
                }
                else {
                    move.loc = my_loc;
                    move.dir = neighDirByLoc.at(queue_entry.first);
                    moves.insert(move);
                    found_neigh = 1;
                    break;
                }
            }
            if(found_neigh)
                continue;
            if(border) {
                move.loc = my_loc;
                move.dir = STILL;
                moves.insert(move);
                continue;
            }

            
            if(my_site.strength < 8 * my_site.production &&
                my_site.strength < 255) {

                move.loc = my_loc;
                move.dir = STILL;
                moves.insert(move);
                continue;
            }

            hlt::Site origin_site = presentMap.getSite(my_loc, my_site.direction);
            hlt::Location origin_loc = presentMap.getLocation(my_loc, my_site.direction);

            if(my_site.strength == 255 && origin_site.strength < 255 && origin_site.production < my_site.production) {
                move.dir = my_site.direction;
                move.loc = my_loc;
                moves.insert(move);
                continue;
            }

            if(origin_site.owner == myID && origin_site.strength + my_site.strength <= 255) {
                move.dir = my_site.direction;
                move.loc = my_loc;
                moves.insert(move);
                continue;
            }

            move.loc = my_loc;
            move.dir = STILL;
            moves.insert(move);
        }

        sendFrame(moves);
    }

    return 0;
}
