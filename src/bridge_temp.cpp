#include "common.h"

#include <stdarg.h>

#ifndef SERVER
#include "client.h"
#endif
#include "server.h"
#include "environment.h"
#include "player.h"
#include "sha1.h"

#ifndef SERVER
static Client *bridge_client = NULL;

void bridge_register_client(Client *c)
{
	bridge_client = c;
}

int bridge_client_send_msg(char* str)
{
	if (!str)
		return 1;
	std::wstring m = narrow_to_wide(str);

	bridge_client->sendChatMessage(m);

	return 0;
}
#endif

int bridge_server_get_status(command_context_t *ctx, char* buff, int size)
{
	if (!ctx)
		return 1;

	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;
	std::string status = wide_to_narrow(server->getStatusString());

	if (snprintf(buff,size,"%s",status.c_str()) >= size)
		return 1;

	return 0;
}

int bridge_server_notify_player(command_context_t *ctx, char* name, char* str, ...)
{
	int l;
	char buff[1024];
	va_list ap;

	if (!ctx)
		return 1;

	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;

	va_start(ap, str);
	l = vsnprintf(buff,1024,str,ap);
	va_end(ap);

	if (l >= 1024)
		return 1;

	std::wstring msg(narrow_to_wide(buff));

	if (name) {
		server->notifyPlayer(name,msg);
	}else{
		server->notifyPlayers(msg);
	}

	return 0;
}

int bridge_server_settime(command_context_t *ctx, uint32_t time)
{
	if (!ctx)
		return 1;

	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;

	server->setTimeOfDay(time);

	return 0;
}

int bridge_server_shutdown(command_context_t *ctx)
{
	if (!ctx)
		return -1;

	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;

	server->requestShutdown();

	return 0;
}

int bridge_server_get_player_ip_or_name(command_context_t *ctx, char* name, char* buff, int size)
{
	if (!ctx)
		return -1;
	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;
	ServerEnvironment *env = static_cast<ServerEnvironment*>(ctx->bridge_env);
	if (!env)
		return -1;

	Player *player = env->getPlayer(name);
	if (player == NULL)
		return -1;

	try{
		Address address = server->getPeerAddress(player->peer_id);
		std::string ip_string = address.serializeString();
		if (ip_string.size() >= (uint32_t)size)
			return -1;
		strcpy(buff,ip_string.c_str());
	} catch(con::PeerNotFoundException) {
		std::string ip_string = ((ServerRemotePlayer*)player)->getAddress();
		if (ip_string == "")
			return -1;
		if (ip_string.size() >= (uint32_t)size)
			return -1;
		strcpy(buff,ip_string.c_str());
	}
	return 0;
}

int bridge_server_add_player(command_context_t *ctx, char* name, char* pass)
{
	if (!ctx)
		return -1;
	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;

	server->addUser(name,pass);

	return 0;
}

int bridge_set_player_password(command_context_t *ctx, char* name, char* pass)
{
	if (!ctx)
		return -1;
	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;

	server->setPlayerPassword(name, pass);

	return 0;
}

int bridge_env_check_player(command_context_t *ctx, char* name)
{
	if (!ctx)
		return -1;

	ServerEnvironment *env = static_cast<ServerEnvironment*>(ctx->bridge_env);
	if (!env)
		return -1;

	Player *tp = env->getPlayer(name);
	if (tp == NULL)
		return 0;

	return 1;
}

int bridge_env_player_pos(command_context_t *ctx, char* name, v3_t *pos)
{
	if (!ctx)
		return -1;

	ServerEnvironment *env = static_cast<ServerEnvironment*>(ctx->bridge_env);
	if (!env)
		return -1;

	Player *tp = env->getPlayer(name);
	if (tp == NULL)
		return 1;

	v3f pp = tp->getPosition();

	pos->x = pp.X;
	pos->y = pp.Y;
	pos->z = pp.Z;

	return 0;
}

int bridge_env_clear_objects(command_context_t *ctx)
{
	if (!ctx)
		return -1;

	ServerEnvironment *env = static_cast<ServerEnvironment*>(ctx->bridge_env);
	if (!env)
		return -1;

	env->clearAllObjects();

	return 0;
}

int bridge_move_player(command_context_t *ctx, v3_t *pos)
{
	if (!ctx)
		return -1;

	Server *server = static_cast<Server*>(ctx->bridge_server);
	if (!server)
		return -1;

	Player *player = static_cast<Player*>(ctx->bridge_player);
	if (!player)
		return 0;

	v3f dest(pos->x,pos->y,pos->z);

	player->setPosition(dest);
	server->SendMovePlayer(player);

	return 0;
}

unsigned char* bridge_sha1(char *str)
{
	int l;

	if (!str || !str[0])
		return NULL;

	l = strlen(str);

	SHA1 sha1;
	sha1.addBytes(str, l);
	return sha1.getDigest();
}

std::string bridge_config_get(char* name)
{
	char* v = config_get(name);
	if (!v)
		return "";

	return std::string(v);
}
