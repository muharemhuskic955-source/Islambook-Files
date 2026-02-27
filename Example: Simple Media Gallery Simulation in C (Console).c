#include <stdio.h>
#include <string.h>

#define MAX_ALBUMS 5
#define MAX_MEDIA_PER_ALBUM 10
#define MAX_DESC_LEN 100

typedef enum {
    IMAGE,
    VIDEO
} MediaType;

typedef struct {
    MediaType type;
    char filename[50];
    char description[MAX_DESC_LEN];
} MediaItem;

typedef struct {
    char name[50];
    char privacy[10]; // e.g., "public", "private"
    MediaItem media[MAX_MEDIA_PER_ALBUM];
    int mediaCount;
} Album;

Album albums[MAX_ALBUMS];
int albumCount = 0;

// Add a new album
void addAlbum(const char* name, const char* privacy) {
    if (albumCount >= MAX_ALBUMS) {
        printf("Max albums reached.\n");
        return;
    }
    strcpy(albums[albumCount].name, name);
    strcpy(albums[albumCount].privacy, privacy);
    albums[albumCount].mediaCount = 0;
    albumCount++;
    printf("Album '%s' created with privacy '%s'.\n", name, privacy);
}

// Add media to album
void addMediaToAlbum(int albumIndex, MediaType type, const char* filename, const char* description) {
    if (albumIndex < 0 || albumIndex >= albumCount) {
        printf("Invalid album index.\n");
        return;
    }
    Album* album = &albums[albumIndex];
    if (album->mediaCount >= MAX_MEDIA_PER_ALBUM) {
        printf("Album '%s' is full.\n", album->name);
        return;
    }
    MediaItem* item = &album->media[album->mediaCount];
    item->type = type;
    strncpy(item->filename, filename, sizeof(item->filename) - 1);
    strncpy(item->description, description, sizeof(item->description) - 1);
    album->mediaCount++;
    printf("Added %s '%s' to album '%s'.\n", type == IMAGE ? "image" : "video", filename, album->name);
}

// Display albums and media
void displayGallery() {
    printf("\n--- Media Gallery ---\n");
    for (int i = 0; i < albumCount; i++) {
        Album* album = &albums[i];
        printf("Album %d: %s (Privacy: %s)\n", i + 1, album->name, album->privacy);
        if (album->mediaCount == 0) {
            printf("  No media in this album.\n");
        } else {
            for (int j = 0; j < album->mediaCount; j++) {
                MediaItem* item = &album->media[j];
                printf("  %d. [%s] %s - %s\n", j + 1, item->type == IMAGE ? "Image" : "Video", item->filename, item->description);
            }
        }
        printf("\n");
    }
}

int main() {
    addAlbum("Vacation 2025", "public");
    addAlbum("Family", "private");

    addMediaToAlbum(0, IMAGE, "beach.jpg", "Sunny beach");
    addMediaToAlbum(0, VIDEO, "surfing.mp4", "Surfing video");
    addMediaToAlbum(1, IMAGE, "family_photo.jpg", "Family reunion");

    displayGallery();

    return 0;
}
