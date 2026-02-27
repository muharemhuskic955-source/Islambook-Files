#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_CONNECTIONS 10

typedef struct User {
    char username[20];
    struct User* friends[MAX_CONNECTIONS];
    int friendCount;
    struct User* followers[MAX_CONNECTIONS];
    int followerCount;
    struct User* following[MAX_CONNECTIONS];
    int followingCount;
    struct User* blocked[MAX_CONNECTIONS];
    int blockedCount;
    bool friendRequests[MAX_USERS]; // friendRequests[i] = true if user i sent request
} User;

User users[MAX_USERS];
int userCount = 0;

// Find user index by username
int findUserIndex(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

// Add user
void addUser(const char* username) {
    if (userCount >= MAX_USERS) {
        printf("Max users reached.\n");
        return;
    }
    strcpy(users[userCount].username, username);
    users[userCount].friendCount = 0;
    users[userCount].followerCount = 0;
    users[userCount].followingCount = 0;
    users[userCount].blockedCount = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        users[userCount].friendRequests[i] = false;
    }
    userCount++;
}

// Check if user1 blocked user2
bool isBlocked(User* user1, User* user2) {
    for (int i = 0; i < user1->blockedCount; i++) {
        if (user1->blocked[i] == user2) return true;
    }
    return false;
}

// Send friend request from sender to receiver
void sendFriendRequest(User* sender, User* receiver) {
    if (isBlocked(receiver, sender)) {
        printf("%s is blocked by %s. Cannot send friend request.\n", sender->username, receiver->username);
        return;
    }
    int receiverIndex = findUserIndex(receiver->username);
    if (receiver->friendRequests[receiverIndex]) {
        printf("Friend request already sent.\n");
        return;
    }
    receiver->friendRequests[findUserIndex(sender->username)] = true;
    printf("%s sent a friend request to %s.\n", sender->username, receiver->username);
}

// Accept friend request
void acceptFriendRequest(User* receiver, User* sender) {
    int senderIndex = findUserIndex(sender->username);
    if (!receiver->friendRequests[senderIndex]) {
        printf("No friend request from %s to accept.\n", sender->username);
        return;
    }
    // Add each other as friends
    if (receiver->friendCount < MAX_CONNECTIONS && sender->friendCount < MAX_CONNECTIONS) {
        receiver->friends[receiver->friendCount++] = sender;
        sender->friends[sender->friendCount++] = receiver;
        printf("%s and %s are now friends.\n", receiver->username, sender->username);
    } else {
        printf("Friend list full.\n");
    }
    receiver->friendRequests[senderIndex] = false;
}

// Follow user
void followUser(User* follower, User* followee) {
    if (isBlocked(followee, follower)) {
        printf("%s is blocked by %s. Cannot follow.\n", follower->username, followee->username);
        return;
    }
    // Check if already following
    for (int i = 0; i < follower->followingCount; i++) {
        if (follower->following[i] == followee) {
            printf("%s already follows %s.\n", follower->username, followee->username);
            return;
        }
    }
    if (follower->followingCount < MAX_CONNECTIONS && followee->followerCount < MAX_CONNECTIONS) {
        follower->following[follower->followingCount++] = followee;
        followee->followers[followee->followerCount++] = follower;
        printf("%s started following %s.\n", follower->username, followee->username);
    } else {
        printf("Follow limit reached.\n");
    }
}

// Unfollow user
void unfollowUser(User* follower, User* followee) {
    bool found = false;
    for (int i = 0; i < follower->followingCount; i++) {
        if (follower->following[i] == followee) {
            // Remove from following list
            for (int j = i; j < follower->followingCount - 1; j++) {
                follower->following[j] = follower->following[j + 1];
            }
            follower->followingCount--;
            found = true;
            break;
        }
    }
    if (!found) {
        printf("%s does not follow %s.\n", follower->username, followee->username);
        return;
    }
    // Remove follower from followee's followers
    for (int i = 0; i < followee->followerCount; i++) {
        if (followee->followers[i] == follower) {
            for (int j = i; j < followee->followerCount - 1; j++) {
                followee->followers[j] = followee->followers[j + 1];
            }
            followee->followerCount--;
            break;
        }
    }
    printf("%s unfollowed %s.\n", follower->username, followee->username);
}

// Block user
void blockUser(User* blocker, User* blockee) {
    if (blocker->blockedCount >= MAX_CONNECTIONS) {
        printf("Block list full.\n");
        return;
    }
    blocker->blocked[blocker->blockedCount++] = blockee;
    printf("%s blocked %s.\n", blocker->username, blockee->username);
    // Remove friend connection if exists
    for (int i = 0; i < blocker->friendCount; i++) {
        if (blocker->friends[i] == blockee) {
            for (int j = i; j < blocker->friendCount - 1; j++) {
                blocker->friends[j] = blocker->friends[j + 1];
            }
            blocker->friendCount--;
            break;
        }
    }
    for (int i = 0; i < blockee->friendCount; i++) {
        if (blockee->friends[i] == blocker) {
            for (int j = i; j < blockee->friendCount - 1; j++) {
                blockee->friends[j] = blockee->friends[j + 1];
            }
            blockee->friendCount--;
            break;
        }
    }
}

// Print user connections
void printUserConnections(User* user) {
    printf("User: %s\n", user->username);
    printf("Friends (%d): ", user->friendCount);
    for (int i = 0; i < user->friendCount; i++) {
        printf("%s ", user->friends[i]->username);
    }
    printf("\nFollowers (%d): ", user->followerCount);
    for (int i = 0; i < user->followerCount; i++) {
        printf("%s ", user->followers[i]->username);
    }
    printf("\nFollowing (%d): ", user->followingCount);
    for (int i = 0; i < user->followingCount; i++) {
        printf("%s ", user->following[i]->username);
    }
    printf("\nBlocked (%d): ", user->blockedCount);
    for (int i = 0; i < user->blockedCount; i++) {
        printf("%s ", user->blocked[i]->username);
    }
    printf("\n");
}

// Check if two users are mutual friends
bool areMutualFriends(User* u1, User* u2) {
    bool u1HasU2 = false, u2HasU1 = false;
    for (int i = 0; i < u1->friendCount; i++) {
        if (u1->friends[i] == u2) u1HasU2 = true;
    }
    for (int i = 0; i < u2->friendCount; i++) {
        if (u2->friends[i] == u1) u2HasU1 = true;
    }
    return u1HasU2 && u2HasU1;
}

int main() {
    // Create users
    addUser("Alice");
    addUser("Bob");
    addUser("Charlie");

    User* alice = &users[findUserIndex("Alice")];
    User* bob = &users[findUserIndex("Bob")];
    User* charlie = &users[findUserIndex("Charlie")];

    // Simulate friend requests and follows
    sendFriendRequest(alice, bob);
    acceptFriendRequest(bob, alice);

    followUser(charlie, alice);
    followUser(bob, alice);

    blockUser(alice, charlie);

    printUserConnections(alice);
    printUserConnections(bob);
    printUserConnections(charlie);

    // Check mutual friends
    printf("Are Alice and Bob mutual friends? %s\n", areMutualFriends(alice, bob) ? "Yes" : "No");
    printf("Are Alice and Charlie mutual friends? %s\n", areMutualFriends(alice, charlie) ? "Yes" : "No");

    return 0;
}
