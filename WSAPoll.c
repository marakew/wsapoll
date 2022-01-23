//WSAIoctl handler the same for
//SIO_BASE_HANDLE
//SIO_BSP_HANDLE
//SIO_BSP_HANDLE_SELECT
//SIO_BSP_HANDLE_POLL
//SOCKET_ERROR / WSAEFAULT

//
// Structures for IOCTL_AFD_POLL.
//

typedef struct _AFD_POLL_HANDLE_INFO {
    HANDLE Handle;
    ULONG PollEvents;
    NTSTATUS Status;
} AFD_POLL_HANDLE_INFO, *PAFD_POLL_HANDLE_INFO;

typedef struct _AFD_POLL_INFO {
    LARGE_INTEGER Timeout;
    ULONG NumberOfHandles;
    BOOLEAN Unique;
    AFD_POLL_HANDLE_INFO Handles[1];
} AFD_POLL_INFO, *PAFD_POLL_INFO;

#define AFD_POLL_RECEIVE_BIT            0
#define AFD_POLL_RECEIVE                (1 << AFD_POLL_RECEIVE_BIT)
#define AFD_POLL_RECEIVE_EXPEDITED_BIT  1
#define AFD_POLL_RECEIVE_EXPEDITED      (1 << AFD_POLL_RECEIVE_EXPEDITED_BIT)
#define AFD_POLL_SEND_BIT               2
#define AFD_POLL_SEND                   (1 << AFD_POLL_SEND_BIT)
#define AFD_POLL_DISCONNECT_BIT         3
#define AFD_POLL_DISCONNECT             (1 << AFD_POLL_DISCONNECT_BIT)
#define AFD_POLL_ABORT_BIT              4
#define AFD_POLL_ABORT                  (1 << AFD_POLL_ABORT_BIT)
#define AFD_POLL_LOCAL_CLOSE_BIT        5
#define AFD_POLL_LOCAL_CLOSE            (1 << AFD_POLL_LOCAL_CLOSE_BIT)
#define AFD_POLL_CONNECT_BIT            6
#define AFD_POLL_CONNECT                (1 << AFD_POLL_CONNECT_BIT)
#define AFD_POLL_ACCEPT_BIT             7
#define AFD_POLL_ACCEPT                 (1 << AFD_POLL_ACCEPT_BIT)
#define AFD_POLL_CONNECT_FAIL_BIT       8
#define AFD_POLL_CONNECT_FAIL           (1 << AFD_POLL_CONNECT_FAIL_BIT)
#define AFD_POLL_QOS_BIT                9
#define AFD_POLL_QOS                    (1 << AFD_POLL_QOS_BIT)
#define AFD_POLL_GROUP_QOS_BIT          10
#define AFD_POLL_GROUP_QOS              (1 << AFD_POLL_GROUP_QOS_BIT)

#define AFD_NUM_POLL_EVENTS             11
#define AFD_POLL_ALL                    ((1 << AFD_NUM_POLL_EVENTS) - 1)


#define FSCTL_AFD_BASE                  FILE_DEVICE_NETWORK
#define _AFD_CONTROL_CODE(request,method) \
                ((FSCTL_AFD_BASE)<<12 | (request<<2) | method)
#define _AFD_REQUEST(ioctl) \
                ((((ULONG)(ioctl)) >> 2) & 0x03FF)
#define _AFD_BASE(ioctl) \
                ((((ULONG)(ioctl)) >> 12) & 0xFFFFF)


#define AFD_POLL                    9

#define IOCTL_AFD_POLL                    _AFD_CONTROL_CODE( AFD_POLL, METHOD_BUFFERED )

#define POLL_STATIC_SIZE 4

static INT NtStatusToSocketError(NTSTATUS Status)
{
	//RtlNtStatusToDosError(Status);

	switch ( Status )
	{
	case STATUS_PENDING:
		return ERROR_IO_PENDING;

	case STATUS_INVALID_HANDLE:
	case STATUS_OBJECT_TYPE_MISMATCH:
		return WSAENOTSOCK;

	case STATUS_INSUFFICIENT_RESOURCES:
	case STATUS_PAGEFILE_QUOTA:
	case STATUS_COMMITMENT_LIMIT:
	case STATUS_WORKING_SET_QUOTA:
	case STATUS_NO_MEMORY: //?
	case STATUS_CONFLICTING_ADDRESSES: //?
	case STATUS_QUOTA_EXCEEDED:
	case STATUS_TOO_MANY_PAGING_FILES:
	case STATUS_REMOTE_RESOURCES:
	case STATUS_TOO_MANY_ADDRESSES:
		return WSAENOBUFS;

	case STATUS_SHARING_VIOLATION:
	case STATUS_ADDRESS_ALREADY_EXISTS:
		return WSAEADDRINUSE;

	case STATUS_LINK_TIMEOUT:
	case STATUS_IO_TIMEOUT:
	case STATUS_TIMEOUT:
		return WSAETIMEDOUT;

	case STATUS_GRACEFUL_DISCONNECT:
		return WSAEDISCON;

	case STATUS_REMOTE_DISCONNECT:
	case STATUS_CONNECTION_RESET: //?
	case STATUS_LINK_FAILED:
	case STATUS_CONNECTION_DISCONNECTED: //?
	case STATUS_PORT_UNREACHABLE:
		return WSAECONNRESET;

	case STATUS_LOCAL_DISCONNECT:
	case STATUS_TRANSACTION_ABORTED:
	case STATUS_CONNECTION_ABORTED:
		return WSAECONNABORTED;

	case STATUS_BAD_NETWORK_PATH:
	case STATUS_NETWORK_UNREACHABLE:
	case STATUS_PROTOCOL_UNREACHABLE:
		return WSAENETUNREACH;

	case STATUS_HOST_UNREACHABLE:
		return WSAEHOSTUNREACH;

	case STATUS_CANCELLED:
	case STATUS_REQUEST_ABORTED: //?
		return WSAEINTR;

	case STATUS_BUFFER_OVERFLOW:
	case STATUS_INVALID_BUFFER_SIZE:
		return WSAEMSGSIZE;

	case STATUS_BUFFER_TOO_SMALL:
	case STATUS_ACCESS_VIOLATION:
	case STATUS_DATATYPE_MISALIGNMENT_ERROR: //+++
		return WSAEFAULT;

	case STATUS_DEVICE_NOT_READY:
	case STATUS_REQUEST_NOT_ACCEPTED:
		return WSAEWOULDBLOCK;

	case STATUS_INVALID_NETWORK_RESPONSE:
	case STATUS_NETWORK_BUSY:
	case STATUS_NO_SUCH_DEVICE:
	case STATUS_NO_SUCH_FILE:
	case STATUS_OBJECT_PATH_NOT_FOUND:
	case STATUS_OBJECT_NAME_NOT_FOUND:
	case STATUS_UNEXPECTED_NETWORK_ERROR:
		return WSAENETDOWN;

	case STATUS_INVALID_CONNECTION:
		return WSAENOTCONN;

	case STATUS_REMOTE_NOT_LISTENING:
	case STATUS_CONNECTION_REFUSED:
		return WSAECONNREFUSED;

	case STATUS_PIPE_DISCONNECTED:
		return WSAESHUTDOWN;

	case STATUS_INVALID_ADDRESS:
	case STATUS_INVALID_ADDRESS_COMPONENT:
		return WSAEADDRNOTAVAIL;

	case STATUS_NOT_SUPPORTED:
	case STATUS_NOT_IMPLEMENTED:
		return WSAEOPNOTSUPP;

	case STATUS_ACCESS_DENIED:
		return WSAEACCES;

	default:
		if (NT_SUCCESS(Status))
		{
			return NO_ERROR;
		}
		return WSAEINVAL;

	case STATUS_UNSUCCESSFUL:
	case STATUS_INVALID_PARAMETER:
	case STATUS_ADDRESS_CLOSED:
	case STATUS_CONNECTION_INVALID:
	case STATUS_ADDRESS_ALREADY_ASSOCIATED:
	case STATUS_ADDRESS_NOT_ASSOCIATED:
	case STATUS_INVALID_DEVICE_STATE:
	case STATUS_INVALID_DEVICE_REQUEST:
		return WSAEINVAL;

	case STATUS_CONNECTION_ACTIVE:
		return WSAEISCONN;

	case STATUS_HOST_DOWN:
		return WSAEHOSTDOWN;

	case STATUS_PROTOCOL_NOT_SUPPORTED:
		return WSAEAFNOSUPPORT;

	case STATUS_HOPLIMIT_EXCEEDED:
		return WSAENETRESET;
	}
}

static ULONG MSAFD_WSPPoll(WSAPOLLFD *fdArray, ULONG fds, int timeout)
{
	NTSTATUS status;
	ULONG error;
	PAFD_POLL_INFO pollInfo;
	ULONG pollBufferSize;
	PAFD_POLL_HANDLE_INFO pollHandleInfo;
	ULONG handleCount;
	ULONG i;
	ULONG n;
	HANDLE eventHandle;
	IO_STATUS_BLOCK ioStatusBlock;
	ULONG handlesReady;

	UCHAR buffer[sizeof(AFD_POLL_INFO)+sizeof(AFD_POLL_HANDLE_INFO)*POLL_STATIC_SIZE];

	if (fdArray == NULL)
	{
		error = WSAEINVAL;
		goto exit;
	}

	if (fds == 0)
	{
		error = 0; //???
		goto exit;
	}

	pollBufferSize = sizeof(AFD_POLL_INFO)+sizeof(AFD_POLL_HANDLE_INFO)*fds;
	if (pollBufferSize > (sizeof(AFD_POLL_INFO)+sizeof(AFD_POLL_HANDLE_INFO)*POLL_STATIC_SIZE))
	{
		pollInfo = (AFD_POLL_INFO *)RtlAllocateHeap(GetProcessHeap(), 0, pollBufferSize);
		if (!pollInfo)
		{
			error = WSAENOBUFS;
			goto exit;
		}
		memset(pollInfo, 0, pollBufferSize);
	} else
	{
		pollInfo = (AFD_POLL_HANDLE_INFO*)&buffer;
		memset(buffer, 0, sizeof(buffer));
	}

	if (timeout < 0)
		pollInfo->Timeout = INT64_MAX;
	else	pollInfo->Timeout = timeout * -10000;

	handleCount = 0;
	ULONG maxnfds = 0;

	for (i = 0; i < fds; ++i)
	{
		fdArray[i].revents = 0;
		if (fdArray[i].fd == INVALID_HANDLE)
		{
			fdArray[i].revents = POLLNVAL;
		} else
		if ( (fdArray[i].events & ~(POLLRDBAND|POLLRDNORM|POLLWRNORM)) != 0 )
		{
			error_code = WSAEINVAL;
			goto exit;
		} else
		{
			handleCount = maxnfds;
			BOOL found = FALSE;
			n = 0;

			pollHandleInfo = pollInfo->Handles;

			while (n < maxnfds)
			{
				if (pollHandleInfo->Handle == fdArray[i].fd)
				{
					found = TRUE;
					break;
				}
				++pollHandleInfo;
				++n;
			}
			pollHandleInfo->Handle = fdArray[i].fd;
			if ( (fdArray[i].events & POLLRDNORM) != 0 )
				pollHandleInfo->PollEvents |= AFD_POLL_ACCEPT|AFD_POLL_RECEIVE;
			if ( (fdArray[i].events & POLLRDBAND) != 0 )
				pollHandleInfo->PollEvents |= AFD_POLL_RECEIVE_EXPEDITED;
			if ( (fdArray[i].events & POLLWRNORM) != 0 )
				pollHandleInfo->PollEvents |= AFD_POLL_CONNECT_FAIL|AFD_POLL_SEND; //++win10fix
			pollHandleInfo->PollEvents |= AFD_POLL_ABORT|AFD_POLL_DISCONNECT;

			if (!found)
				handleCount = ++maxnfds;
		}
	}
	
	if (handleCount == 0)
	{
		error = WSAEINVAL;
		goto exit;
	}

	pollInfo->NumberOfHandles = handleCount;
	pollInfo->Unique = FALSE;

	status = NtDeviceIoControlFile(pollInfo->Handles[0].Handle,
					NULL, //??? TODO
					NULL,
					NULL,
					&ioStatusBlock,
					IOCTL_AFD_POLL, //0x12024
					pollInfo, pollBufferSize,
					pollInfo, pollBufferSize);
	if (status == STATUS_PENDING)
	{
		//??? TODO
	}

	if (!NT_SUCCESS(status))
	{
		error = NtStatusToSocketError(status);
		goto exit;
	}

	for (i = 0; i < fds; ++i)
	{
		pollHandleInfo = pollInfo->Handles;

		for (n = 0; n < pollInfo->NumberOfHandles; ++n)
		{
			if (pollHandleInfo->Handle == fdArray[i].fd)
			{
				if ( (pollHandleInfo->PollEvents & (AFD_POLL_ACCEPT|AFD_POLL_RECEIVE)) != 0 &&
					(fdArray[i].events & POLLRDNORM) == POLLRDNORM )
				{
					fdArray[i].revents |= POLLRDNORM;
				}

				if ( (pollHandleInfo->PollEvents & AFD_POLL_RECEIVE_EXPEDITED) != 0 &&
					(fdArray[i].events & POLLRDBAND) == POLLRDBAND )
				{
					fdArray[i].revents |= POLLRDBAND;
				}

				if ( (pollHandleInfo->PollEvents & (AFD_POLL_CONNECT_FAIL|AFD_POLL_SEND)) != 0 && //++win10fix
					(fdArray[i].events & POLLWRNORM) != 0) // POLLOUT
					fdArray[i].revents |= POLLWRNORM; // POLLOUT

				if ( (pollHandleInfo->PollEvents & AFD_POLL_DISCONNECT) != 0 )
					fdArray[i].revents |= POLLHUP;

				if ( (pollHandleInfo->PollEvents & (AFD_POLL_CONNECT_FAIL|AFD_POLL_ABORT)) != 0 ) //++win10fix
					fdArray[i].revents |= POLLHUP|POLLERR;

				if ( (pollHandleInfo->PollEvents & AFD_POLL_LOCAL_CLOSE) != 0 )
					fdArray[i].revents |= POLLNVAL;
        		}
			++pollHandleInfo;
		}
	}

	handlesReady = 0;
	for (i = 0; i < fds; ++i)
	{
		if ( (fdArray[j].revents & (POLLRDBAND|POLLRDNORM|POLLWRNORM|POLLHUP)) != 0 )
			++handlesReady;
	}

	handlesReady = pollInfo->NumberOfHandles;
	
exit:
	if (pollInfo && pollInfo != &buffer)
		RtlFreeHeapFree(GetProcessHeap(), 0, pollInfo);

	SetLastError(error);
	return error ? SOCKET_ERROR : handlesReady;
}


int WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout)
{
	ULONG i;
	ULONG error = NO_ERROR;

	LPWSAPOLLFD pollInfo = NULL;

	if (fds == 0)
	{
		error = WSAEINVAL;
		goto exit;
	}

	ULONG pollInfoBufferSize = sizeof(WSAPOLLFD) * fds;
	pollInfo = HeapAlloc(GetProcessHeap(), 0, pollInfoBufferSize);
	if (!pollInfo)
	{
		error = WSAENOBUFS;
		goto exit;
	}
	memcpy(pollInfo, fdArray, pollInfoBufferSize);

	BOOL found = FALSE;
	SOCKET foundfd = INVALID_SOCKET;

	for (i = 0; i < fds; ++i)
	{
		SOCKET nativefd = INVALID_SOCKET;
		DWORD BytesReturned = 0;
		SOCKET fd = pollInfo[i].fd;
		if (fd == INVALID_SOCKET)
		{
			fdArray[i].revents = POLLNVAL;
		} else
		if (WSAIoctl(fd, SIO_BSP_HANDLE_POLL, NULL, NULL, &nativefd, sizeof(nativefd), &BytesReturned, NULL, NULL) == SOCKET_ERROR)
		{
			fdArray[i].revents = POLLNVAL;
		} else
		{
			if (!found)
			{
				foundfd = nativefd;
				found = TRUE;
			}
			pollInfo[i].fd = nativefd;
		}
	}
	
	if (!found)
	{
		error = WSAENOTSOCK;
		goto exit;
	}

	ULONG result = MSAFD_WSPPoll(pollInfo, fds, timeout);
	if (result == SOCKET_ERROR)
	{
		error = GetLastError();
		goto exit;
	}

	for (i = 0; i < fds; ++i)
	{
		fdArray[i].revents = pollInfo[i].revents;
	}
	HeapFree(GetProcessHeap(), 0, pollInfo);
	return result;
exit:
	if (pollInfo)
		HeapFree(GetProcessHeap(), 0, pollInfo);
	SetLastError(error);
	return SOCKET_ERROR;
}
