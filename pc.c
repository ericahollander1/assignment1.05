#include <stdlib.h>

#include "string.h"

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"

void pc_delete(pc_t *pc)
{
    if (pc) {
        free(pc);
    }
}

uint32_t pc_is_alive(dungeon_t *d)
{
    return d->pc.alive;
}

void place_pc(dungeon_t *d)
{
    d->pc.position[dim_y] = rand_range(d->rooms->position[dim_y],
                                       (d->rooms->position[dim_y] +
                                        d->rooms->size[dim_y] - 1));
    d->pc.position[dim_x] = rand_range(d->rooms->position[dim_x],
                                       (d->rooms->position[dim_x] +
                                        d->rooms->size[dim_x] - 1));
}
void write_monsters(dungeon_t *d, pair_t *list_monsters_pos, int min_value, int max_value){
    int i, j;
    int line = 4;
    int x_dif, y_dif;
    for(j=28; j<50; j++){
        for(i=2; i<27; i++){
            mvprintw(i, j, " ");
        }
    }
    for(i=29; i<50; i++);{
        mvprintw(2, i, "-");
        mvprintw(15, i, "-");
    }
    for(i=2; i<16; i++){
        mvprintw(i, 28, "|");
        mvprintw(i, 50, "|");
    }
    for(i=min_value; i<max_value; i++){
        x_dif = d->pc.position[dim_x]-list_monsters_pos[i][dim_x];

        y_dif = d->pc.position[dim_y]-list_monsters_pos[i][dim_y];
        mvprintw(line, 30, "%c: %2d %s %2d %s", charpair(list_monsters_pos[i])->symbol, abs(x_dif), "monsterx", abs(y_dif), "monstery";//direction_x(x_dif), direction_y(y_dif)
    line++;
    }
    refresh();
}
void display_monsters(dungeon_t *d, pair_t dir){
    int move;
    int done = 0;
    pair_t p;
    pair_t list_monsters_pos[d->num_monsters];
    int current = 0;
    int max_value= 10;
    int min_value = 0;

    for(p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++){
        for(p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++){
            if(charpair(p)){
                if(charpair(p)->symbol == '0'){
                    continue;
                }
                list_monsters_pos[current][dim_y] = p[dim_y];
                list_monsters_pos[current][dim_x] = p[dim_x];
                current++;
            }
        }
    }
    write_monsters(d, list_monsters_pos, min_value, max_value);
    while(!done){
        move = getch();
        switch(move){
            case 27:
                done = 1;
                break;
            case KEY_UP:
                if(min_value != 0){
                    min_value--;
                    max_value--;
                }
                write_monsters(d, list_monsters_pos, min_value, max_value);
                break;
            case KEY_DOWN:
                if(min_value != d->num_monsters-1){
                    min_value++;
                    max_value++;
                }
                break;
            case 'Q':
            case'q':
                endwin();
                exit(0);
                break;
        }
    }
    render_dungeon(d);
}

void config_pc(dungeon_t *d)
{
    memset(&d->pc, 0, sizeof (d->pc));
    d->pc.symbol = '@';

    place_pc(d);

    d->pc.speed = PC_SPEED;
    d->pc.alive = 1;
    d->pc.sequence_number = 0;
    d->pc.pc = calloc(1, sizeof (*d->pc.pc));
    d->pc.npc = NULL;
    d->pc.kills[kill_direct] = d->pc.kills[kill_avenged] = 0;

    d->character[d->pc.position[dim_y]][d->pc.position[dim_x]] = &d->pc;

    dijkstra(d);
    dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
//    int no_op;
//    int mon_list=0;
    char key;
    dir[dim_y]=dir[dim_x]=0;

    //while(!dead){
    key = getch();
    switch(key){
        case '7':
        case 'y':
            dir[dim_y]--;
            dir[dim_x]--;
            break;
        case '8':
        case 'k':
            dir[dim_y]--;
            break;
        case '9':
        case 'u':
            dir[dim_y]--;
            dir[dim_x]++;
            break;
        case '6':
        case 'l':
            dir[dim_y]++;
            break;
        case '3':
        case 'n':
            dir[dim_y]++;
            dir[dim_x]++;
            break;
        case '2':
        case 'j':
            dir[dim_y]++;
            break;
        case '1':
        case 'b':
            dir[dim_y]++;
            dir[dim_x]--;
            break;
        case '4':
        case 'h':
            dir[dim_x]--;
            break;
        case '>':
            if(d->map[d->pc.position[dim_y]][d->pc.position[dim_x]] == ter_stairs_down){
                delete_dungeon(d);
                init_dungeon(d);
                gen_dungeon(d);
                config_pc(d);
                gen_monsters(d);
                render_dungeon(d);
            }
            dir[dim_y] = dir[dim_x] = 0;
            break;
        case '<':
            if(d->map[d->pc.position[dim_y]][d->pc.position[dim_x]] == ter_stairs_up){
                delete_dungeon(d);
                init_dungeon(d);
                gen_dungeon(d);
                config_pc(d);
                gen_monsters(d);
                render_dungeon(d);
            }
            dir[dim_y] = dir[dim_x] = 0;
            break;
        case ' ':
        case '5':
        case '.':
            break;
        case 27:
            pc_next_pos(d, dir);
            break;
        case 'm':
            display_monsters(d, dir);
            break;
            // case KEY_DOWN:
            //      break;
            //  case KEY_UP:
            //     break;
            //case KEY_EXIT:
            //   mon_list = 0;
            //  break;
        case 'Q':
            endwin();
            exit(0);
            break;
        default:
            mvprintw(21, 0, "key: %c", key);
            refresh();
            break;
    }
    return 0;
}

uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
    if ((room < d->num_rooms)                                     &&
        (d->pc.position[dim_x] >= d->rooms[room].position[dim_x]) &&
        (d->pc.position[dim_x] < (d->rooms[room].position[dim_x] +
                                  d->rooms[room].size[dim_x]))    &&
        (d->pc.position[dim_y] >= d->rooms[room].position[dim_y]) &&
        (d->pc.position[dim_y] < (d->rooms[room].position[dim_y] +
                                  d->rooms[room].size[dim_y]))) {
        return 1;
    }

    return 0;
}
