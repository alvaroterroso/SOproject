#include "mobile_user.h"

int main(int argc, char **argv){
	if(argc < 7){
		printf("Wrong input command!\nUsage: mobile_user <plafond inicial> "
		       "<número de pedidos de autorização> <intervalo VIDEO> "
		       "<intervalo MUSIC> <intervalo SOCIAL> <dados a reservar>\n");
		exit(1);
	}
	mobile_user_struct *mobile_user = malloc(sizeof(mobile_user_struct));
	for(int i = 1; i < 7; i++){
		if(atoi(argv[i]) < 1){
			printf("INPUT ERROR!\n%d must be < 0", atoi(argv[i]));
			exit(1);
		}
	}
	
	mobile_user->init_plafond = atoi(argv[1]);
	mobile_user->auth_request_number =atoi(argv[2]);
	mobile_user->video_interval = atoi(argv[3]);
	mobile_user->music_interval = atoi(argv[4]);
	mobile_user->social_interval = atoi(argv[5]);
	mobile_user->to_reserve_data = atoi(argv[6]);

	printf("Mobile user data as been saved in mobile_user struct\n");
	free(mobile_user);
	return 0;
}

