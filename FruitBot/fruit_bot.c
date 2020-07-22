// fruit_bot.c
// Fruit Bot
//
// Given information about the bot and the world that it's in, this program tries
// to print the move that will result in the highest profit
//
// This program by Stratton Sloane on 03-06-2018
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "fruit_bot.h"

// Directions
#define EAST 1
#define WEST -1

// Bot actions
#define NOTHING 0
#define BUY 1
#define SELL 2
#define CHARGE 3

// Information returned by traverse() function,
// describing a location relative to the start
struct traverse_result {
    struct location *loc;
    int distance;
    int direction;

    struct location *next_e;
    struct location *next_w;
    struct location *start;
};

// Used for storing a list of locations
struct loc_list {
    struct location *loc;
    struct loc_list *next;
};

void print_player_name(void);
void print_move(struct bot *b);
void run_unit_tests();

// Unit tests
void test_setup_functions();
void test_location_info();
void test_world_information();

// Getting information about a location
int is_selling(struct location *loc);
int is_buying(struct location *loc);
int is_selling_item(struct location *loc, char *item);
int is_buying_item(struct location *loc, char *item);
int has_fruit(struct location *loc);
int has_electricity(struct location *loc);
int get_num_bot_sellers(struct location *loc);
int get_kg_bot_sellers(struct location *loc);
int get_num_bot_buyers(struct location *loc);
int get_kg_bot_buyers(struct location *loc);

struct traverse_result *start_traverse(struct location *start_location);
void traverse(struct traverse_result *previous);

// Nearest locations to buy/sell items
struct location *nearest_seller_item(struct location *loc, char *item);
struct location *nearest_buyer_item(struct location *loc, char *item);
struct location *nearest_electricity(struct location *loc);

// Amount to buy/sell at a location, and the profit that can be made
int get_max_purchase(struct bot *b, struct location *loc);
int get_max_sale(int fruit_kg, struct location *loc);
int get_profit(struct bot *b, struct location *loc);
int get_max_charge(struct bot *b, struct location *loc);

// Getting the best locations in the world to buy and sell fruit
struct location *get_best_seller(struct bot *b);
struct location *get_best_buyer(struct bot *b);
struct location *get_best_buyer_loc(struct location *loc, char *fruit, int fruit_kg,
                                 int maximum_move, struct loc_list *excluded);

struct location *get_location(struct location *start, char *name);
int get_distance(struct location *current, struct location *dest);
int get_direction(struct location *current, struct location *dest);
int get_move(struct location *current, struct location *dest, int max_move);
int get_num_turns(struct location *current, struct location *dest, int max_move);

// Creating and testing for membership in a location list
void add_to_loc_list(struct loc_list *head, struct location *loc);
int in_loc_list(struct loc_list *head, struct location *loc);

// Unit testing helper functions
struct location *create_test_location(char *name, char *fruit, 
                                      int price, int quantity);
struct location *add_location(struct location *loc, char *name, char *fruit,
                              int price, int quantity);
void link_locations(struct location* loc1, struct location* loc2);
struct bot *create_test_bot();
struct bot *add_test_bot(struct location *loc, char *fruit, int kg);
void add_test_bots(struct location *loc, char *fruit, int kg, int num);
struct location *create_test_world1();


int main(int argc, char *argv[]) {

    if (argc > 1) {
        // supply any command-line argument to run unit tests
        run_unit_tests();
        return 0;
    }

    struct bot *me = fruit_bot_input(stdin);
    if (me == NULL) {
        print_player_name();
    } else {
        print_move(me);
    }

    return 0;
}

void print_player_name(void) {
    // CHANGE THIS PRINTF TO YOUR DESIRED PLAYER NAME
    printf("Nans Banananans");

}

// print_move - should print a single line indicating
//              the move your bot wishes to make
//
// This line should contain only the word Move, Sell or Buy
// followed by a single integer

void print_move(struct bot *b) {

    int action = NOTHING;
    struct location *target = NULL;

    if (b->fruit_kg == 0) {
        action = BUY;  
    } else if (b->fruit_kg > 0) {
        action = SELL;
    }

    if (action == SELL) {
        target = get_best_buyer(b);

        // If target = NULL, there's no one to buy our fruit
        if (target == NULL) {
            action = BUY;
        } 
    }

    if (action == BUY) {
        target = get_best_seller(b);

        // If no seller is returned, none are profitable
        if (target == NULL) {
            action = NOTHING;
        }
    }

    struct location *electricity = nearest_electricity(b->location);
    int charge_threshold = 0;
    charge_threshold += get_distance(b->location, electricity); // Can get to charge point
    charge_threshold += get_distance(b->location, target); // Can get back from target
    charge_threshold += b->maximum_move; // Extra margin

    // In smaller worlds calculated threshold can be too high
    if (charge_threshold > b->battery_capacity / 2) {
        charge_threshold = b->battery_capacity / 2;
    }

    if (b->battery_level < charge_threshold) {
        target = electricity;
        action = CHARGE;
    }

    if (action == NOTHING) {
        printf("Move 0\n");
    } else {
        // If we're already at the target, we should buy or sell
        if (target == b->location) {
            if (action == BUY) {
                // Try to buy as many as we can afford
                int buy_amount = get_max_purchase(b, target);
                printf("Buy %d\n", buy_amount);
            } else if (action == SELL) {
                // Sell as many as we can
                int sell_amount = b->fruit_kg;
                printf("Sell %d\n", sell_amount);
            } else if (action == CHARGE) {
                // Try to buy as much as we can afford, up to 85% of our cash
                int buy_amount = get_max_charge(b, target);
                printf("Buy %d\n", buy_amount);
            }
        // Otherwise, move towards the target
        } else {
            int move = get_move(b->location, target, b->maximum_move);

            printf("Move %d\n", move);
        }
    }
}

// Write unit tests for functions beforehand
// Start with basic functions, chained together to form complex functions
// Separate related groups of unit tests into separate functions

void run_unit_tests() {
    test_setup_functions();
    test_location_info();
    test_world_information();
}


// Test determining information about a location (eg, if it is selling)

void test_location_info() {
    // Location buying mangos
    struct location *mango_buyer;
    mango_buyer = create_test_location("Mango Tango", "Mango", 70, 12);

    assert(is_buying(mango_buyer) == 1);
    assert(is_selling(mango_buyer) == 0);
    assert(is_buying_item(mango_buyer, "Mango") == 1);
    assert(is_selling_item(mango_buyer, "Mango") == 0);
    assert(is_buying_item(mango_buyer, "Banana") == 0);
    assert(is_selling_item(mango_buyer, "Banana") == 0);
    assert(has_electricity(mango_buyer) == 0);
    assert(has_fruit(mango_buyer) == 1);
    mango_buyer->quantity = 0;
    assert(has_fruit(mango_buyer) == 0);
    
    // Location selling electricity
    struct location *elec_seller;
    elec_seller = create_test_location("Chargin Today", "Electricity", -5, 100);

    assert(is_buying(elec_seller) == 0);
    assert(is_selling(elec_seller) == 1);
    assert(is_buying_item(elec_seller, "Mango") == 0);
    assert(is_selling_item(elec_seller, "Mango") == 0);
    assert(has_electricity(elec_seller) == 1);
    assert(has_fruit(elec_seller) == 0);

    // Test maximum purchase we can make  
    struct bot *b = create_test_bot();
    struct location *mango_seller;
    mango_seller = create_test_location("Mango Tango", "Mango", -70, 30);
    
    // Maximum we can afford
    b->maximum_fruit_kg = 30;
    b->cash = 1430;
    assert(get_max_purchase(b, mango_seller) == 20);
    b->cash = 515; 
    assert(get_max_purchase(b, mango_seller) == 7);
    b->cash = 1131;
    assert(get_max_purchase(b, mango_seller) == 16);

    // Maximum we can carry
    b->cash = 10000;
    b->maximum_fruit_kg = 15;
    assert(get_max_purchase(b, mango_seller) == 15); 
    b->maximum_fruit_kg = 1;
    assert(get_max_purchase(b, mango_seller) == 1); 

    // Maximum purchase when other buying bots are present
    struct bot *bot1 = add_test_bot(mango_seller, NULL, 0);
    struct bot *bot2 = add_test_bot(mango_seller, NULL, 0);
    struct bot *bot3 = add_test_bot(mango_seller, NULL, 0);
    // Each bot can afford to buy 10
    bot1->cash = 700;
    bot1->maximum_fruit_kg = 30;
    bot2->cash = 700;
    bot2->maximum_fruit_kg = 30;
    bot3->cash = 700;
    bot3->maximum_fruit_kg = 30;
    assert(get_max_purchase(bot1, mango_seller) == 10);
    mango_seller->quantity = 28; 
    assert(get_max_purchase(bot1, mango_seller) == 9); // 9 split evenly
    mango_seller->quantity = 3;
    assert(get_max_purchase(bot1, mango_seller) == 1);
    mango_seller->quantity = 2;
    assert(get_max_purchase(bot1, mango_seller) == 0); // Can't split evenly

    // Maximum sale
    mango_buyer->quantity = 10;
    int bot_kg;
    bot_kg = 8;
    assert(get_max_sale(bot_kg, mango_buyer) == 8); // We only have 8
    bot_kg = 15;
    assert(get_max_sale(bot_kg, mango_buyer) == 10); // They only have 10
    bot_kg = 0;
    assert(get_max_sale(bot_kg, mango_buyer) == 0); // We have nothing to sell
    bot_kg = 10;
    assert(get_max_sale(bot_kg, mango_buyer) == 10); // Perfect

    // Check bots are present
    add_test_bots(mango_buyer, "Mango", 2, 5);
    add_test_bots(mango_buyer, "Apple", 10, 1);
    add_test_bots(mango_buyer, "Durian", 12, 1);

    assert(get_num_bot_sellers(mango_buyer) == 5); // 5 bots with mango
    assert(get_kg_bot_sellers(mango_buyer) == 10); // Holding 10 kg in total 

    // Maximum sale when bots are present
    mango_buyer->bots = NULL;
    mango_buyer->quantity = 50;
    add_test_bots(mango_buyer, "Mango", 10, 3); // 3 bots with 10 kg each
    b = mango_buyer->bots->bot; // We are one of the three bots

    assert(get_max_sale(b->fruit_kg, mango_buyer) == 10);  // Total kg of bots under quantity
    mango_buyer->quantity = 30; 
    assert(get_max_sale(b->fruit_kg, mango_buyer) == 10); // Still enough for all bots
    mango_buyer->quantity = 23; 
    assert(get_max_sale(b->fruit_kg, mango_buyer) == 7); // Not enough, but split evenly
    mango_buyer->quantity = 2; 
    assert(get_max_sale(b->fruit_kg, mango_buyer) == 0); // Can't split between 3, so 0

}

// Test functions used to get information about a world (eg, distances, directions, best locations)

void test_world_information() {
    struct location *start = create_test_world1();
    struct location *other1 = get_location(start, "Other1");
    struct location *apple_buyer1 = get_location(start, "AppleBuyer1");
    struct location *other2 = get_location(start, "Other2");
    struct location *elec_seller = get_location(start, "ElecSeller");
    struct location *apple_seller = get_location(start, "AppleSeller");
    struct location *any_buyer = get_location(start, "AnyBuyer");
    struct location *apple_buyer2 = get_location(start, "AppleBuyer2");

    struct traverse_result *t = start_traverse(other1);

    assert(t->loc == other1);
    assert(t->distance == 0);

    // Traverse should return first location to the east
    traverse(t);
    assert(t->loc == apple_buyer1);
    assert(t->distance == 1);
    assert(t->direction == EAST);
    assert(strcmp(t->loc->fruit, "Apples") == 0);

    // Then fist location to the west
    traverse(t);
    assert(t->loc == apple_buyer2);
    assert(t->distance == 1);
    assert(t->direction == WEST);

    // Nearest sellers
    assert(nearest_seller_item(other1, "Electricity") == elec_seller);
    assert(nearest_seller_item(elec_seller, "Electricity") == elec_seller);
    assert(nearest_seller_item(other1, "Apples") == apple_seller);
    assert(nearest_seller_item(apple_seller, "Apples") == apple_seller);

    // Nearest buyers
    assert(nearest_buyer_item(other1, "Apples") == apple_buyer1);
    assert(nearest_buyer_item(apple_buyer1, "Apples") == apple_buyer1);
    assert(nearest_buyer_item(apple_seller, "Apples") == apple_buyer2);
    assert(nearest_buyer_item(elec_seller, "Anything") == any_buyer);
    assert(nearest_buyer_item(other1, "Jackfruit") == NULL);

    // Distances between two locations
    assert(get_distance(other1, apple_buyer1) == 1); // First location east
    assert(get_distance(other2, any_buyer) == 3); // Further east
    assert(get_distance(any_buyer, apple_seller) == 1); // Heading west
    assert(get_distance(elec_seller, other1) == 3); // Further west
    assert(get_distance(elec_seller, elec_seller) == 0); // Same location
    assert(get_distance(other1, apple_buyer2) == 1); // Rollover west
    assert(get_distance(apple_buyer2, other1) == 1); // Rollover east

    // Directions
    assert(get_direction(other1, apple_buyer1) == EAST);
    assert(get_direction(other1, apple_buyer2) == WEST);
    assert(get_direction(elec_seller, other1) == WEST);

    // Getting required movement
    int max_move = 5;
    assert(get_move(other1, elec_seller, max_move) == 3); // Moving east
    assert(get_move(elec_seller, other1, max_move) == -3); // Moving west

    max_move = 2;
    assert(get_move(other1, elec_seller, max_move) == 2); // Limited by max move
    assert(get_move(elec_seller, other1, max_move) == -2);

    // Getting required turns
    max_move = 5;
    assert(get_num_turns(other2, any_buyer, max_move) == 1);
    assert(get_num_turns(apple_seller, apple_buyer1, max_move) == 1);
    max_move = 2;
    assert(get_num_turns(other2, any_buyer, max_move) == 2);
    assert(get_num_turns(apple_seller, apple_buyer1, max_move) == 2);
    max_move = 1;
    assert(get_num_turns(other2, any_buyer, max_move) == 3);
    assert(get_num_turns(apple_seller, apple_buyer1, max_move) == 3);

    struct bot *b = create_test_bot();
    b->location = other1;
    b->fruit = malloc(sizeof("Apples"));
    b->maximum_move = 3;
    b->cash = 100;
    b->maximum_fruit_kg = 50;
    b->turns_left = 100;
    strcpy(b->fruit, "Apples");

    assert(get_best_seller(b) == apple_seller);
    b->fruit_kg = 5;
    // Apple buyer 2 is buying at a lower price, but buying 3 fruit
    assert(get_best_buyer(b) == apple_buyer2);
    b->fruit_kg = 1;
    // Apple buyer 1 is buying 1 fruit at a higher price
    assert(get_best_buyer(b) == apple_buyer1);

    apple_buyer1->quantity = 0;
    assert(get_best_buyer(b) == apple_buyer2);

    apple_buyer2->quantity = 0;
    assert(get_best_buyer(b) == any_buyer);

    any_buyer->quantity = 0;
    assert(get_best_buyer(b) == NULL);

    b->turns_left = 0;
    assert(get_best_seller(b) == NULL);
}

// Test functions used to setup unit tests

void test_setup_functions() {
    // Create a test location

    struct location *mango_buyer;

    mango_buyer = create_test_location("Mango Tango", "Mango", 70, 12);
    assert(strcmp(mango_buyer->name, "Mango Tango") == 0);
    assert(strcmp(mango_buyer->fruit, "Mango") == 0);
    assert(mango_buyer->price == 70);
    assert(mango_buyer->quantity == 12);

    // Link two locations together

    struct location *apple_seller;
    apple_seller = create_test_location("Yapp Apple", "Apple", -24, 5);

    link_locations(mango_buyer, apple_seller);
    assert(mango_buyer->east == apple_seller);
    assert(apple_seller->west == mango_buyer);
    assert(mango_buyer->west == NULL);
    assert(apple_seller->east == NULL);

    // Link three locations in a circle

    struct location *any_buyer;
    any_buyer = add_location(mango_buyer, "Dumpyer's Tuf", "Anything", 1, 1000);

    link_locations(any_buyer, mango_buyer);
    assert(any_buyer->west == apple_seller);
    assert(any_buyer->east == mango_buyer);
    assert(mango_buyer->west == any_buyer);
    assert(apple_seller->east == any_buyer);

    // Add bots to a location

    add_test_bot(mango_buyer, "Mango", 5);
    struct bot_list *bots = mango_buyer->bots;
    assert(bots != NULL);
    struct bot *b1 = bots->bot;
    assert(b1 != NULL);
    assert(b1->fruit != NULL);
    assert(strcmp(b1->fruit, "Mango") == 0);
    assert(b1->fruit_kg == 5);
    assert(bots->next == NULL);

    add_test_bot(mango_buyer, "Apples", 10);
    assert(bots->next != NULL);
    struct bot *b2 = bots->next->bot;
    assert(b2 != NULL);
    assert(b2->fruit != NULL);
    assert(strcmp(b2->fruit, "Apples") == 0);
    assert(bots->next->next == NULL);
}

// Returns whether a location has something to sell

int is_selling(struct location *loc) {
    if (loc->price < 0 && loc->quantity > 0) {
        return 1;
    }

    return 0;
}

// Returns whether a location wants to buy something

int is_buying(struct location *loc) {
    if (loc->price > 0 && loc->quantity > 0) {
        return 1;
    }

    return 0;
}

// Returns whether a location is selling a given fruit

int is_selling_item(struct location *loc, char *item) {
    if (is_selling(loc) && strcmp(loc->fruit, item) == 0) {
        return 1;
    }

    return 0;
}

// Returns whether a location is buying a given fruit

int is_buying_item(struct location *loc, char *item) {
    if (is_buying(loc) && strcmp(loc->fruit, item) == 0) {
        return 1;
    }

    return 0;
}

// Returns whether a location has fruit to buy or sell

int has_fruit(struct location *loc) {
    if (strcmp("Anything", loc->fruit) == 0 ||
        strcmp("Electricity", loc->fruit) == 0 ||
        strcmp("Nothing", loc->fruit) == 0 ||
        loc->fruit == NULL) {
        return 0;        
    } else if (is_selling(loc) || is_buying(loc)) {
        return 1;
    } else {
        return 0;
    }
}

// Returns whether location has electricity to sell

int has_electricity(struct location *loc) {
    return is_selling_item(loc, "Electricity");
}

// Returns the number of bots at a location that can sell to a location

int get_num_bot_sellers(struct location *loc) {

    struct bot_list *n = loc->bots;
    int count = 0;
    
    while (n != NULL) {
        if (n->bot->fruit != NULL && strcmp(n->bot->fruit, loc->fruit) == 0) {
            count++;
        }
        n = n->next;
    }

    return count;
}


// Returns the total fruit amount that the bots at a location have to sell

int get_kg_bot_sellers(struct location *loc) {

    struct bot_list *n = loc->bots;
    int kg = 0;
    
    while (n != NULL) {
        if (n->bot->fruit != NULL && strcmp(n->bot->fruit, loc->fruit) == 0) {
            kg += n->bot->fruit_kg;
        }
        n = n->next;
    }

    return kg;
}

// Returns the number of bots at a location that can buy from a location

int get_num_bot_buyers(struct location *loc) {

    struct bot_list *n = loc->bots;
    int count = 0;
    
    while (n != NULL) {
        if (n->bot->fruit_kg == 0) {
            count++;
        }
        n = n->next;
    }

    return count;
}


// Returns the total fruit amount that the bots at a location can buy

int get_kg_bot_buyers(struct location *loc) {

    struct bot_list *n = loc->bots;
    int kg = 0;
    int bot_max = 0;
    
    while (n != NULL) {
        if (n->bot->fruit_kg == 0) {
            bot_max = n->bot->cash / abs(loc->price);

            if (bot_max > n->bot->maximum_fruit_kg) {
                bot_max = n->bot->maximum_fruit_kg;
            }

            kg += bot_max;
        }
        n = n->next;
    }

    return kg;
}

// Given a start location, return an initial traverse result

struct traverse_result *start_traverse(struct location *start_location) {
    struct traverse_result *t = malloc(sizeof(struct traverse_result));

    t->loc = start_location;
    t->distance = 0;
    t->direction = 0;
    t->next_e = t->loc->east;
    t->next_w = t->loc->west;
    t->start = start_location;
    
    return t;
}

// Traverse the world, keeping track of direction and distance

void traverse(struct traverse_result *t) {
    
    // Set location to NULL if we've traversed the whole map
    if ((t->distance > 0 && t->loc == t->start) || t->loc == NULL) {
        t->loc = NULL;
    }
    // If last location was start, or last move was west, move east
    else if (t->distance == 0 || t->direction == WEST) {
        t->direction = EAST;
        t->distance += 1;
        t->loc = t->next_e;
        t->next_e = t->loc->east;
    // Otherwise move west
    } else if (t->direction == EAST) {
        t->direction = WEST;
        t->loc = t->next_w;
        t->next_w = t->loc->west;
    }
}

// Returns the nearest seller of an item from a location

struct location *nearest_seller_item(struct location *loc, char *item) {
    struct traverse_result *t = start_traverse(loc);
    struct location *nearest = NULL;

    while (t->loc != NULL && nearest == NULL) {
        if (is_selling_item(t->loc, item)) {
            nearest = t->loc;
        }

        traverse(t);
    }

    free(t);
    return nearest;
}

// Returns the nearest buyer of an item from a location

struct location *nearest_buyer_item(struct location *loc, char *item) {
    struct traverse_result *t = start_traverse(loc);
    struct location *nearest = NULL;

    while (t->loc != NULL && nearest == NULL) {
        if (is_buying_item(t->loc, item)) {
            nearest = t->loc;
        }

        traverse(t);
    }

    free(t);
    return nearest;
}

// Returns the nearest seller of electricity from a location

struct location *nearest_electricity(struct location *loc) {
    return nearest_seller_item(loc, "Electricity");
}

// Calculates the maximum amount of fruit/electricity we can buy at a location

int get_max_purchase(struct bot *b, struct location *loc) {
    int available = loc->quantity;
    int max_affordable = b->cash / abs(loc->price);
    int max_purchase = max_affordable;

    // If there's not enough for all bots, divide available by number of bots
    if (has_fruit(loc) && get_kg_bot_buyers(loc) > available) {
        available = available / get_num_bot_buyers(loc);
    }    

    if (max_purchase > available) {
       max_purchase = available;
    } 

    if (has_fruit(loc) && max_purchase > b->maximum_fruit_kg) {
        max_purchase = b->maximum_fruit_kg;
    }

    return max_purchase;
}

// Calculates the maximum amount of fruit we can sell to a location

int get_max_sale(int fruit_kg, struct location *loc) {
    int available = loc->quantity;

    // If there's more trying to be sold than the location has available
    if (get_kg_bot_sellers(loc) > available) {
        available = available / get_num_bot_sellers(loc);
    }

    if (fruit_kg > available) {
        return available;
    } else {
        return fruit_kg;
    }
}

// Returns the maximum amount of electricity to buy

int get_max_charge(struct bot *b, struct location *loc) {
    // Highest amount ever needed
    int max_needed = b->maximum_move * b->turns_left; 
    // Only use to 85% of cash
    int affordable = (b->cash - (b->cash / 6)) / abs(loc->price);
    int max_charge = b->battery_capacity - b->battery_level;

    if (max_charge > affordable) {
        max_charge = affordable;
    }

    if (max_charge > max_needed) {
        max_charge = max_needed;
    }

    return max_charge;
}

// Calculates the profit we can make from seller, divided by the number of turns

int get_profit(struct bot *b, struct location *loc) {

    int bot_kg, sell_kg, profit, num_turns;
    struct location *seller = loc;
    struct location *prev = seller;

    // Turns to get to seller and buy fruit
    num_turns = get_num_turns(b->location, loc, b->maximum_move) + 1;
    // Set initial amount of fruit and profit 
    bot_kg = get_max_purchase(b, seller);   // The amount of fruit we buy
    profit = bot_kg * seller->price;    // Negative profit

    struct loc_list *excluded = NULL;
    struct location *buyer = get_best_buyer_loc(seller, seller->fruit, bot_kg, 
                                                b->maximum_move, excluded);
    
    // Find best buyers until there are none left, or we run out of fruit, or no turns
    while (buyer != NULL && bot_kg > 0 && num_turns <= b->turns_left) { 
        // Turns required to get there, plus one turn for selling
        num_turns += get_num_turns(prev, buyer, b->maximum_move) + 1;
        
        // Only 'sell' if there are enough turns left
        if (num_turns <= b->turns_left) {
            sell_kg = get_max_sale(bot_kg, buyer);
            bot_kg -= sell_kg;
            profit += sell_kg * buyer->price;
        }

        // If we still have fruit left, the buyer has none, so exclude
        if (excluded == NULL) {
            excluded = malloc(sizeof(struct loc_list));
            excluded->next = NULL;
            excluded->loc = buyer;
        } else {
            add_to_loc_list(excluded, buyer);
        }

        // Get next best buyer, relative to the buyer we just bought from
        prev = buyer;
        buyer = get_best_buyer_loc(buyer, seller->fruit, bot_kg, 
                                    b->maximum_move, excluded);
    }

    return profit / num_turns;
}

// Find the seller with highest profit per turn

struct location *get_best_seller(struct bot *b) {
    struct traverse_result *t_bot = start_traverse(b->location);

    struct location *best_seller = NULL;
    int best_profit = 0;

    int profit, bot_kg, sell_kg;
    struct location *buyer;
    struct location *seller;
    
    while (t_bot->loc != NULL) {

        // If we find a seller that has fruit, calculate the profit
        if(is_selling(t_bot->loc) && has_fruit(t_bot->loc)) {
            seller = t_bot->loc;
            profit = get_profit(b, seller);

            if (profit > best_profit) {
                best_seller = seller;
                best_profit = profit;
            }
        }

        traverse(t_bot);
    }

    free(t_bot);
    return best_seller;
}

// Find the best buyer for the fruit we current have

struct location *get_best_buyer(struct bot *b) {
    return get_best_buyer_loc(b->location, b->fruit, b->fruit_kg, 
                           b->maximum_move, NULL);
}

// Returns the best buyer relative to a location, given fruit and maximum move
// Best = highest profit per turn

struct location *get_best_buyer_loc(struct location *loc, char *fruit, int fruit_kg,
                                 int maximum_move, struct loc_list *excluded) {
    struct location *best = NULL;
    int best_profit = 0;
    int profit, num_turns, profit_turn;
    struct traverse_result *t = start_traverse(loc);

    while (t->loc != NULL) {
        // If the location is a buyer, and they're not on the list of excluded buyers
        if(is_buying_item(t->loc, fruit) && !in_loc_list(excluded, t->loc)) {
            
            profit = get_max_sale(fruit_kg, t->loc) * t->loc->price;
            num_turns = get_num_turns(loc, t->loc, maximum_move) + 1;
            profit_turn = profit / num_turns;

            if (profit_turn > best_profit) {
                best_profit = profit_turn;
                best = t->loc;
            }
        }

        traverse(t);
    }

    free(t);

    if (best == NULL) {
        return nearest_buyer_item(loc, "Anything");
    } else {
        return best;
    }
}

// Returns the location with the given name

struct location *get_location(struct location *start, char *name) {
    struct traverse_result *t = start_traverse(start);

    while (t->loc != NULL) {
        if (strcmp(t->loc->name, name) == 0) {
            return t->loc;
        }

        traverse(t);
    }

    return NULL;
}

// Return the distance between two locations

int get_distance(struct location *current, struct location *dest) {
    struct traverse_result *t = start_traverse(current);

    // Traverse until we find target destination
    while (t->loc != dest) {
        traverse(t);
    } 

    int distance = t->distance;
    free(t);

    return distance;
}

// Returns the direction of 'dest' relative to 'current'

int get_direction(struct location *current, struct location *dest) {
    struct traverse_result *t = start_traverse(current);

    // Traverse until we find target destination
    while (t->loc != dest) {
        traverse(t);
    } 

    int direction = t->direction;
    free(t);

    return direction;
}

// Return the move required to get to a location, taking into account max move

int get_move(struct location *current, struct location *dest, int max_move) {
    int distance = get_distance(current, dest);
    int direction = get_direction(current, dest);

    if (distance > max_move) {
        return max_move * direction;
    } else {
        return distance * direction;
    }
}

// Returns the number of turns required to get to a location

int get_num_turns(struct location *current, struct location *dest, int max_move) {
    int distance = get_distance(current, dest);
    int turns = distance / max_move + (distance % max_move != 0);

    return turns;
}

// Adds a location to a location list

void add_to_loc_list(struct loc_list *head, struct location *loc) {

    struct loc_list *n = head;

    while (n->next != NULL) {
        n = n->next;
    }

    n->next = malloc(sizeof(struct loc_list));
    n->next->loc = loc;
    n->next->next = NULL;
}

// Returns whether a location is in a location list

int in_loc_list(struct loc_list *head, struct location *loc) {
    if (head == NULL) {
        return 0;
    } 

    struct loc_list *n = head;
    int found = 0;    

    while (n != NULL && !found) {
        if (n->loc == loc) {
            found = 1;
        }

        n = n->next;
    }

    return found;
}

// Creates a location for unit testing

struct location *create_test_location(char *name, char *fruit, 
                                      int price, int quantity) {
    struct location *loc = malloc(sizeof(struct location));

    // Allocate and copy location name
    loc->name = calloc(MAX_NAME_CHARS, sizeof(char));
    strcpy(loc->name, name);

    // Allocate and copy fruit name
    loc->fruit = calloc(MAX_NAME_CHARS, sizeof(char));
    strcpy(loc->fruit, fruit);

    // Set other variables
    loc->price = price;
    loc->quantity = quantity;
    loc->east = NULL;
    loc->west = NULL;
    loc->bots = NULL;

    return loc;
}

// Creates a new location and adds it to the existing one

struct location *add_location(struct location *start, char *name, char *fruit,
                              int price, int quantity) {
    struct location *new = create_test_location(name, fruit, price, quantity);
    struct location *n;
    struct location *prev = start;

    if (start->east == NULL) {
        n = start;
    } else {
        n = start->east;    
    }

    while (n != NULL && n != start) {
        prev = n;
        n = n->east;   
    } 

    link_locations(new, start);
    link_locations(prev, new);
    return new;
}

// Links two locations together for unit testing

void link_locations(struct location* loc1, struct location* loc2) {
    loc1->east = loc2;
    loc2->west = loc1;
}

// Creates a bot for unit testing

struct bot *create_test_bot() {
    struct bot *b = malloc(sizeof(struct bot));

    b->name = NULL;
    b->location = NULL;
    b->fruit = NULL;
    b->cash = 0;
    b->battery_level = 0;
    b->fruit_kg = 0;
    b->turns_left = 0;
    b->battery_capacity = 0;
    b->maximum_move = 0;
    b->maximum_fruit_kg = 0;

    return b;
}

// Adds a bot with the specified fruit to a location

struct bot *add_test_bot(struct location *loc, char *fruit, int kg) {

    struct bot_list *n = loc->bots;
    struct bot_list *prev;

    while (n != NULL) {
        prev = n;
        n = n->next;
    }

    n = malloc(sizeof(struct bot_list));
    n->next = NULL;
    n->bot = create_test_bot();

    if (fruit == NULL) {
        n->bot->fruit = NULL;
    } else {
        n->bot->fruit = malloc(MAX_NAME_CHARS);
        strcpy(n->bot->fruit, fruit);
    }

    n->bot->fruit_kg = kg;

    if (loc->bots == NULL) {
        loc->bots = n;
    } else {
        prev->next = n;
    }

    return n->bot;
}

// Adds a number of bots with a specified fruit to a location

void add_test_bots(struct location *loc, char *fruit, int kg, int num) {
    for (int i = 0; i < num; i++) {
        add_test_bot(loc, fruit, kg);
    }
}

// Creates test world 1 and returns a pointer to the first location

struct location *create_test_world1() {

    struct location *start = create_test_location("Other1", "Nothing", 0, 0);
    struct location *apple_buyer1 = add_location(start, "AppleBuyer1", "Apples", 51, 1);
    struct location *other2 = add_location(start, "Other2", "Lemons", 1, 1);
    struct location *elec_seller = add_location(start, "ElecSeller", "Electricity", -4, 100);
    struct location *apple_seller = add_location(start, "AppleSeller", "Apples", -18, 3);
    struct location *any_buyer = add_location(start, "AnyBuyer", "Anything", 1, 1000);
    struct location *apple_buyer2 = add_location(start, "AppleBuyer2", "Apples", 43, 3);

    return start;
}
