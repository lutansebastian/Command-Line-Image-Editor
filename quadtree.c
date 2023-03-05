#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct pixel_t{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} pixel_t;

typedef struct picture_t{
    int width;
    int height;
    pixel_t **matrix;

} picture_t;

typedef struct node{
    struct node *top_left, *top_right;
    struct node *bottom_left, *bottom_right;
    int size; //dimensiunea patratului
    uint8_t red_avg, green_avg, blue_avg;
} node;

typedef struct QuadtreeNode {
    unsigned char blue, green, red;
    uint32_t area;
    int32_t top_left, top_right;
    int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode;

int height(struct node* node);
void generate_level_tree(struct node* root, int level, QuadtreeNode *array,
                         int *index);

pixel_t initialize_values(unsigned char red, unsigned char green,
unsigned char blue){
    
    pixel_t pixel;
    pixel.red = red;
    pixel.blue = blue;
    pixel.green = green;

    return pixel;
}

picture_t *generate_picture(int height, int width){
    int i;

    picture_t *picture = malloc(sizeof(*picture));
    picture->height = height;
    picture->width = width;

    picture->matrix = malloc(picture->height * sizeof(*(picture->matrix)));

    for (i = 0; i < picture->height; i++){
        picture->matrix[i] = malloc(picture->width *
        sizeof(*(*(picture->matrix))));
    }

    return picture;
}

uint64_t compute_mean(picture_t *picture, int x_start, int x_stop, int y_start,
int y_stop){
    
    int i, j;
    uint64_t sum = 0;
    uint64_t red_avg = 0;
    uint64_t green_avg = 0;
    uint64_t blue_avg = 0;;

    for (i = x_start; i < x_stop; i++){
        for (j = y_start; j < y_stop; j++){
           red_avg = red_avg + picture->matrix[i][j].red;
           green_avg = green_avg + picture->matrix[i][j].green;
           blue_avg = blue_avg + picture->matrix[i][j].blue;
        }
    }
    
    red_avg = red_avg / ((x_stop - x_start) * (y_stop - y_start));
    green_avg = green_avg / ((x_stop - x_start) * (y_stop - y_start));
    blue_avg = blue_avg / ((x_stop - x_start) * (y_stop - y_start));

    for (i = x_start; i < x_stop; i++){
        for (j = y_start; j < y_stop; j++){
            sum = sum + ((red_avg - picture->matrix[i][j].red) *
            (red_avg - picture->matrix[i][j].red) +
            (green_avg - picture->matrix[i][j].green) *
            (green_avg - picture->matrix[i][j].green) +
            (blue_avg - picture->matrix[i][j].blue) *
            (blue_avg - picture->matrix[i][j].blue));
        }
    }
    
    uint64_t mean = 0;
    mean = sum / (3 * (x_stop - x_start) * (x_stop - x_start));

    return mean;
}

void build_tree(node **root, picture_t *picture, int x_start,
                int x_stop, int y_start, int y_stop, uint32_t *nr_nodes,
                int factor_compresie, uint32_t *nr_leaves){
    int i, j;

    if (*root == NULL){
        *root = malloc(sizeof(node));
        if (!*root) {
            printf("error Malloc\n");
        }
        (*root)->top_left = NULL;
        (*root)->top_right = NULL;
        (*root)->bottom_left = NULL;
        (*root)->bottom_right = NULL;
        (*nr_nodes) += 1;
    }

    int red_total = 0;
    int green_total = 0;
    int blue_total = 0;;

    for (i = x_start; i < x_stop; i++){
        for (j = y_start; j < y_stop; j++){
           red_total = red_total + picture->matrix[i][j].red;
           green_total = green_total + picture->matrix[i][j].green;
           blue_total = blue_total + picture->matrix[i][j].blue;
        }
    }

    (*root)->size = (x_stop - x_start) * (x_stop - x_start);
    (*root)->red_avg = red_total / (*root)->size;
    (*root)->green_avg = green_total / (*root)->size;
    (*root)->blue_avg = blue_total / (*root)->size;

    uint64_t mean = compute_mean(picture, x_start, x_stop, y_start, y_stop);

    if (mean <= factor_compresie) {
        (*nr_leaves)++;
        return;
    }

    build_tree(&(*root)->top_left, picture, x_start, ((x_start + x_stop) / 2),
               y_start, ((y_start + y_stop) / 2), nr_nodes, factor_compresie,
               nr_leaves);
    
    build_tree(&(*root)->top_right, picture, x_start, ((x_start + x_stop) / 2),
               ((y_start + y_stop) / 2), y_stop, nr_nodes, factor_compresie,
               nr_leaves);
    
    build_tree(&(*root)->bottom_left, picture, ((x_start + x_stop) / 2), x_stop,
               y_start, ((y_start + y_stop) / 2), nr_nodes, factor_compresie,
               nr_leaves);
    
    build_tree(&(*root)->bottom_right, picture, ((x_start + x_stop) / 2),
               x_stop, ((y_start + y_stop) / 2), y_stop, nr_nodes,
               factor_compresie, nr_leaves);

}

void generate_array(struct node* root, QuadtreeNode *array){
    int h = height(root);
    int i;
    int index = 0;
    for (i = 1; i <= h; i++) {
        generate_level_tree(root, i, array, &index);
    }
}

void generate_level_tree(struct node* root, int level,
                         QuadtreeNode *array, int *index){
    if (root == NULL) {
        return;
    }if (level == 1) {
        array[*index].red = root->red_avg;
        array[*index].green = root->green_avg;
        array[*index].blue = root->blue_avg;
        array[*index].area = root->size;
        if (root->top_left) {
            array[*index].top_left = (*index) * 4 + 1;
            array[*index].top_right = (*index) * 4 + 2;
            array[*index].bottom_right = (*index) * 4 + 3;
            array[*index].bottom_left = (*index) * 4 + 4;
        } else {
            array[*index].top_left = -1;
            array[*index].top_right = -1;
            array[*index].bottom_left = -1;
            array[*index].bottom_right = -1;
        }
        (*index) += 1;
    
    } else if (level > 1) {
        generate_level_tree(root->top_left, level - 1, array, index);
        generate_level_tree(root->top_right, level - 1, array, index);
        generate_level_tree(root->bottom_right, level - 1, array, index);
        generate_level_tree(root->bottom_left, level - 1, array, index);
    }
}

int height(struct node* node){
    int heights[4];
    
    if (node == NULL)
        return 0;
    else {
        heights[0] = height(node->top_left);
        heights[1] = height(node->top_right);
        heights[2] = height(node->bottom_left);
        heights[3] = height(node->bottom_right);
 
        int i;
        int max_heigh = heights[0];
        for (i = 1; i < 4; i++) {
            if (heights[i] > max_heigh) {
                max_heigh = heights[i];
            }
        }
        return max_heigh + 1;
    }
}

void free_tree(node **root){
    if ((*root) != NULL){
        free_tree(&(*root)->top_left);
        free_tree(&(*root)->top_right);
        free_tree(&(*root)->bottom_left);
        free_tree(&(*root)->bottom_right);
        free((*root));
    }
}

int main(int argc, char *argv[]) {
    char input_file[20];
    char output_file[20];
    char buffer[50];

    int factor_compresie = 0;
    int previous_pos;

    if (argv[1][1] == 'c' || argv[1][1] == 'd') {
        factor_compresie = atoi(&argv[1][3]);

        memcpy(input_file, &argv[1][3 + 1 + strlen(&argv[1][3])], 20);
        previous_pos = 3 + 1 + (int)strlen(&argv[1][3]);
        
        memcpy(output_file, &argv[1][previous_pos + 1 +
        strlen(&argv[1][previous_pos])], 20);
    }
    else if (argv[1][1] == 'm'){

        factor_compresie = atoi(&argv[1][5]);

        memcpy(input_file, &argv[1][5 + 1 + strlen(&argv[1][5])], 20);
        previous_pos = 5 + 1 + (int)strlen(&argv[1][5]);
        
        memcpy(output_file, &argv[1][previous_pos + 1 +
        strlen(&argv[1][previous_pos])], 20);
    }

    FILE *file_ptr;
    file_ptr = fopen(input_file, "rb");

    if (file_ptr == NULL){
        printf("Error input file\n");
        exit(1);
    }

    fgets(buffer, 50, file_ptr);

    int width, height, color_value;

    fgets(buffer, 50, file_ptr);
    sscanf(buffer, "%d %d", &width, &height);
    fgets(buffer, 50, file_ptr);
    sscanf(buffer, "%d", &color_value);

    picture_t *picture = generate_picture(height, width);

    int i, j;

    for (i = 0; i < picture->height; i++){
        for (j = 0; j < picture->width; j++){
            fread(&picture->matrix[i][j].red, sizeof(char), 1,
            file_ptr);
            fread(&picture->matrix[i][j].green, sizeof(char), 1,
            file_ptr);
            fread(&picture->matrix[i][j].blue, sizeof(char), 1,
            file_ptr);
        }
    }
    fclose(file_ptr);

    FILE *file_ptr_output;
    file_ptr_output = fopen("output_test_matrix", "wb");

    if (file_ptr_output == NULL){
        printf("Error output file\n");
        exit(1);
    }

    for (i = 0; i < picture->height; i++){
        for (j = 0; j < picture->width; j++){
            fwrite(&picture->matrix[i][j].red, sizeof(char), 1,
            file_ptr_output);
            fwrite(&picture->matrix[i][j].green, sizeof(char), 1,
            file_ptr_output);
            fwrite(&picture->matrix[i][j].blue, sizeof(char), 1,
            file_ptr_output);
        }
    }
    fclose(file_ptr_output);

    if (argv[1][1] == 'c'){

        node *root = NULL;
        uint32_t nr_nodes = 0;
        uint32_t nr_leaves = 0;

        int x_start = 0;
        int x_stop = picture->height;

        int y_start = 0;
        int y_stop = picture->width;

        build_tree(&root, picture, x_start, x_stop, y_start, y_stop, &nr_nodes,
         factor_compresie, &nr_leaves);
        
        QuadtreeNode *array = malloc(nr_nodes * sizeof(QuadtreeNode));

        generate_array(root, array);
        
        
        FILE *write_ptr = fopen(output_file,"wb");

        
        fwrite(&nr_leaves, sizeof(uint32_t), 1, write_ptr);
        fwrite(&nr_nodes, sizeof(uint32_t), 1, write_ptr);
        
        fwrite(array,sizeof(QuadtreeNode),nr_nodes,write_ptr);
        
        fclose(write_ptr);
        free(array);
        free_tree(&root);
        
    }
    else if (argv[1][1] == 'd'){
        printf("Flag : -d\n");
    }
    else if (argv[1][1] == 'm'){
        if (argv[1][3] == 'v'){
            printf("Flag : -m v\n");
        }
        else if (argv[1][3] == 'h'){
            printf("Flag : -m h\n");
        }
    }

    return 0;
}
