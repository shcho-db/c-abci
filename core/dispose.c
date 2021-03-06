/*********************************************************************************************/
/*                   dispose.c Created by xuli on 2017/08/25                                 */
/*                     本文件主要处理每个新的连接                                            */
/*********************************************************************************************/
#include "internal.h"
 
static int handle_response(struct socket_tcp *tsocket, Types__Response *response);

static int handle_request(Application app, Types__Request *request, Types__Response *response, size_t size);


int dispose_connect(struct socket_tcp *tsocket, Application app)
{
	size_t size = 0;
	size_t bytes = 0;
	size_t length = 0;
	struct pollfd pollfd =  {};

	pollfd.fd = tsocket->fd;
	pollfd.events = POLLIN;

	while( 1 )
	{
		if ( poll(&pollfd, 1, POLLTIMEOUT) <= 0 )
            continue ;

        if ( !(pollfd.revents & POLLIN) )
            continue ;

		uint8_t *data = NULL;
		Types__Request *request = NULL;
		Types__Response response = {};

		length = readsize(tsocket, READTIMEOUT);
		if ( length == -1 )
		{
			printf("socket closed or receive data error. fd:%d\n", tsocket->fd);
			return -1;
		}

	    data = (uint8_t*)malloc(length);
		if ( data == NULL )
		{
			printf("no more memory, malloc error.");
			return -1;
		}
		memset(data, 0, sizeof(length));

		bytes = socket_read(tsocket, data, length, READTIMEOUT);
		if ( bytes != length )
		{
			return -1;
		}

		types__response__init(&response);
		request = types__request__unpack(NULL, length, data);
		free(data);
		size = types__request__get_packed_size(request);
		printf("request:length:%lu size:%lu", length, size);

		handle_request(app, request, &response, length);

		handle_response(tsocket, &response);

		types__request__free_unpacked(request, NULL);
	}
	return 0;
}

static int handle_request(Application app, Types__Request *request, Types__Response *response, size_t size)
{
	switch( request->value_case )
	{
		case TYPES__REQUEST__VALUE_ECHO:
			printf("TYPES__REQUEST__VALUE_ECHO\n");
			ToResponseEcho(request, response);
			break ;
		case TYPES__REQUEST__VALUE_FLUSH:
			printf("TYPES__REQUEST__VALUE_FLUSH:\n");
			ToResponseFlush(request, response);
			break ;
		case TYPES__REQUEST__VALUE_INFO:
			{
				printf("TYPES__REQUEST__VALUE_INFO\n");
				Types__ResponseInfo *info = NULL;
				info = (Types__ResponseInfo*)app(request);
				ToResponseInfo(request, response, info);
			}
			break ;
		case TYPES__REQUEST__VALUE_SET_OPTION:
			{
				printf("TYPES__REQUEST__VALUE_OPTION\n");
				Types__ResponseSetOption *setoption = NULL;
				setoption = (Types__ResponseSetOption*)app(request);
				ToResponseSetOption(request, response, setoption);
			}
			
			break ;
		case TYPES__REQUEST__VALUE_DELIVER_TX:
			{
				printf("TYPES__REQUEST__VALUE_DELIVER_TX\n");
				Types__ResponseDeliverTx *delivertx = NULL;
				delivertx = (Types__ResponseDeliverTx*)app(request);
				ToResponseDeliverTx(request, response, delivertx);
			}
			break ;
		case TYPES__REQUEST__VALUE_CHECK_TX:
			{
				printf("TYPES__REQUEST__VALUE_CHECK_TX\n");
				Types__ResponseCheckTx *checktx = NULL;
				checktx = (Types__ResponseCheckTx*)app(request);
				ToResponseCheckTx(request, response, checktx);
			}
			break ;
		case TYPES__REQUEST__VALUE_COMMIT:
			{
				printf("TYPES__REQUEST__VALUE_COMMIT\n");
				Types__ResponseCommit *commit = NULL;
				commit = (Types__ResponseCommit*)app(request);
				ToResponseCommit(request, response, commit);
			}
			break ;
		case TYPES__REQUEST__VALUE_QUERY:
			{
				printf("TYPES__REQUEST__VALUE_QUERY\n");
				Types__ResponseQuery *query = NULL;
				query = (Types__ResponseQuery*)app(request);
				ToResponseQuery(request, response, query);
			}
			break ;
		case TYPES__REQUEST__VALUE_INIT_CHAIN:
			{
				printf("TYPES__REQUEST__VALUE_INIT_CHAIN\n");
				Types__ResponseInitChain *initchain = NULL;
				initchain = (Types__ResponseInitChain*)app(request);
				ToResponseInitChain(request, response, initchain);
			}
			break ;
		case TYPES__REQUEST__VALUE_BEGIN_BLOCK:
			{
				printf("TYPES__REQUEST__VALUE_BEGIN_BLOCK\n");
				Types__ResponseBeginBlock *beginblock = NULL;
				beginblock = (Types__ResponseBeginBlock*)app(request);
				ToResponseBeginBlock(request, response, beginblock);
			}
			break ;
		case TYPES__REQUEST__VALUE_END_BLOCK:
			{
				printf("TYPES__REQUEST__VALUE_END_BLOCK\n");
				Types__ResponseEndBlock *endblock = NULL;
				endblock = (Types__ResponseEndBlock*)app(request);
				ToResponseEndBlock(request, response, endblock);
			}
			break ;
		default:
			printf("TYPES__REQUEST__VALUE__NOT_SET\n");
			ToResponseException(request, response);
			return -1;
	}
	return 0;
}

static int handle_response(struct socket_tcp *tsocket, Types__Response *response)
{
	size_t size = 0;
	size_t bytes = 0;
	uint8_t data[1024] = {};

	if ( tsocket == NULL || response == NULL )
	{
		return -1;
	}

	size = types__response__pack(response, data);

//	size = types__response__get_packed_size(response);

	if ( sendsize(tsocket, size, SENDTIMEOUT) == -1 )
		return -1;

	bytes = socket_send(tsocket, data, size, SENDTIMEOUT);
	if ( bytes != size )
	{
		printf("send data failed.\n");
	}

	switch( response->value_case )
	{
		case TYPES__RESPONSE__VALUE_ECHO:
			response_free_echo(response->echo);
			printf("fd:%d RESPONSE_ECHO send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_FLUSH:
			response_free_flush(response->flush);
			printf("fd:%d RESPONSE_FLUSH send size:%lu\n", tsocket->fd, size);
		//	flush();
			break ;
		case TYPES__RESPONSE__VALUE_INFO:
			response_free_info(response->info);
			printf("fd:%d RESPONSE_INFO send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_SET_OPTION:
			response_free_setoption(response->set_option);
			printf("fd:%d RESPONSE_SET_OPTION send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_DELIVER_TX:
			response_free_delivertx(response->deliver_tx);
			printf("fd:%d RESPONSE_DELIVER_TX send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_CHECK_TX:
			response_free_checktx(response->check_tx);
			printf("fd:%d RESPONSE_CHECK_TX send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_COMMIT:
			response_free_commit(response->commit);
			printf("fd:%d RESPONSE_COMMIT send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_QUERY:
			response_free_query(response->query);
			printf("fd:%d RESPONSE_QUERY send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_INIT_CHAIN:
			response_free_initchain(response->init_chain);
			printf("fd:%d RESPONSE_INIT_CHAIN send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_BEGIN_BLOCK:
			response_free_beginblock(response->begin_block);
			printf("fd:%d RESPONSE_BEGIN_BLOCK send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_END_BLOCK:
			response_free_endblock(response->end_block);
			printf("fd:%d RESPONSE_END_BLOCK send size:%lu\n", tsocket->fd, size);
			break ;
		case TYPES__RESPONSE__VALUE_EXCEPTION:
			response_free_exception(response->exception);
			printf("fd:%d RESPONSE_EXCEPTION send size:%lu\n", tsocket->fd, size);
		default:
			return -1;
	}

	return 0;
}
