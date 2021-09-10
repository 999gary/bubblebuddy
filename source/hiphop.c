
//Created from this documentation :)
//https://heavyironmodding.org/wiki/HIP_(File_Format)

//yeah

#ifdef HIPHOP_IMPLEMENTATION
#define HIPHOPAPI

typedef struct {
    char block_id[4];
    int block_size;
} hh_block;

typedef struct {
    hh_block block;
} section_hipa;

typedef struct {
    hh_block block;
} section_DICT;

typedef struct {
    hh_block block;
    hh_block block_pver;
    int sub_version;
    int client_version;
    int compatible;
    hh_block block_pflg; //useless
    int flags;
    hh_block block_pcnt;
    int asset_count;
    int layer_count;
    int max_asset_size;
    int max_layer_size;
    int max_x_form_asset_size;
    hh_block block_pcrt;
    int created_date;
    char* created_date_string;
    hh_block block_pmod;
    int modified_date;
    hh_block block_plat;
    int platform_id;
    //bfbb only
    char* platform_name;
    char* region;
    char* language;
    char* game_name;
} section_pack;

typedef struct {
    char* filename;
    int filesize;
    int seeklocation;
    char* bytes;
} hhf;

typedef struct {
    hhf file;
    section_hipa hipa;
    section_pack pack;
} hh;

int32_t hh_read_int32(hh* hh)
{
    hh->file.seeklocation += 4;
    return ((hh->file.bytes[hh->file.seeklocation-4] << 24) | 
            (hh->file.bytes[hh->file.seeklocation-3] << 16) | 
            (hh->file.bytes[hh->file.seeklocation-2] << 8) | 
            (hh->file.bytes[hh->file.seeklocation-1]));
}

uint32_t hh_read_uint32(hh* hh)
{
    hh->file.seeklocation += 4;
    return ((hh->file.bytes[hh->file.seeklocation-4] << 24) | 
            (hh->file.bytes[hh->file.seeklocation-3] << 16) | 
            (hh->file.bytes[hh->file.seeklocation-2] << 8) | 
            (hh->file.bytes[hh->file.seeklocation-1]));
}

char* hh_string_pointer(hh* hh)
{
    char* string = hh->file.bytes + hh->file.seeklocation;
    //printf("Bytes: %s", string);
    size_t length = strlen(string);
    hh->file.seeklocation += length;
    if(length % 2)
    {
        hh->file.seeklocation++;
    }
    else
    {
        hh->file.seeklocation += 2;
    }
    return string;
}

void hh_read_block_header(hh* hh, hh_block* block)
{
    strcpy(block->block_id, hh_string_pointer(hh));
    block->block_size = hh_read_int32(hh);
}

int read_hipa(hh* hh)
{
    if(hh->file.seeklocation != 0)
        return 1;
    
    char* magic_number = hh_string_pointer(hh);
    
    if(strncmp(magic_number, "HIPA", 4))
        return 1;
    
    hh_read_block_header(hh, &hh->hipa.block);
}

int read_pack(hh* hh)
{
    if(hh->file.seeklocation == 0)
        return 1; //?????
    
    hh_read_block_header(hh, &hh->pack.block);
    hh_read_block_header(hh, &hh->pack.block_pver);
    hh->pack.sub_version = hh_read_int32(hh);
    hh->pack.client_version = hh_read_int32(hh);
    
}

int hh_read_file(hh* hh)
{
    read_hipa(hh);
    read_pack(hh);
}


//TODO: Put this elsewhere.
HIPHOPAPI
int hh_read_file_from_disk(hh* hh, char* path)
{
    //strcpy(hh->file.filename, path);
    printf("Reading file with path: %s\n", path);
    FILE* file = fopen(path, "rb");
    if (file == NULL)
        return 1;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    hh->file.bytes = malloc(size + 1);
    fseek(file, 0, SEEK_SET);
    fread(hh->file.bytes, 1, size, file);
    hh->file.bytes[size] = '\0';
    fclose(file);
    return hh_read_file(hh);
}

//TODO: Actually have a use for this.
/*
HIPHOPAPI
void hh_read_file_from_memory(char* bytes, uint32_t size)
{

}
*/
#endif