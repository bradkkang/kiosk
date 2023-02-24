#ifndef _TMAX_WSGW_USER_H_
#define _TMAX_WSGW_USER_H_

/* 데이터 변환 함수 */
/* 환경 설정 적용 
   configfile [in] : 웹서비스 환경설정 파일 경로
   metafile [in] : 웹서비스 정보 파일 경로
   */
int init_config(char *configfile, char *metafile);

/* tmax msg를 soap msg 로 전환하는 함수 
   svcname [in] : 서비스 이름
   tmaxmsg [in] : 변환될 tmax msg
   soapmsg [out] : 이 함수 안에서 memory alloc 시킴
   olen [out] : soap msg 길이
   subtype [in] : tmaxmsg가 구성된 struct 이름
   uri [in] : 웹서비스 제공자의 URI
   soapversion [in] : 변환될 soap msg 의 version (1.1 or 1.2)
   encodingstyle [in] : 변환될 soap msg 의 encoding type (rpc or doc)
   wsns [in] : 변환될 soap msg 의 NameSpace
   typens [in] : 변환될 soap msg 의 type NameSpace 
   */
int convert_tmax_to_soap(char *svcname, char *tmaxmsg, char **soapmsg, int *olen, 
		char *subtype, char *uri, char *soapversion, char*encodingstyle, 
		char *wsns, char *typens );
/* soap msg를 tmax msg 로 전환하는 함수 
   soapmsg [in] : 변환될 tmax msg
   soapmsglen [in] : soap msg의 길이
   tmaxmsg [out] : 이 함수 안에서 memory alloc 시킴
   len [out] : tmaxmsg 의 길이
   */
int convert_soap_to_tmax(char *soapmsg, int soapmsglen, char **tmaxmsg, int *len);


/* 웹서비스 게이트웨이에서 호출되는 callback 함수 */
/* tmax -> 외부 로 요청이 호출됨 
   tmaxmsg [in] : 변환될 tmaxmsg
   ilen [in] : tmaxmsg 의 길이
   soapmsg [out] : 변환된 soapmsg - 내부에서 free하기 때문에 반드시 malloc으로 할당해야 한다.
   olen [out] : 변환된 soapmsg 의 길의
   service [out] : 실제 서비스 이름 - 내부에서 free하기 때문에 반드시 malloc으로 할당해야 한다.
   uri [out] : uri - 내부에서 free하기 때문에 반드시 malloc으로 할당해야 한다.
   soap_version [out] : soap version ( 1.1 or 1.2 ) - 내부에서 free하기 때문에 반드시 malloc으로 할당해야 한다.

   */
int (*request_from_tmax)(char *tmaxmsg, int ilen, char **soapmsg, int *olen , char **service, char **uri, char **soap_version);

/* 외부 -> tmax로의 요청에 대한 응답이 있을 경우 호출되는 callback 함수
   tmaxmsg [in] : 변환될 tmaxmsg
   soapmsg [out] : 변환된 soapmsg
   olen [out] : 변환된 soapmsg 의 길의
   service [out] : 실제 서비스 이름
   */
int (*reply_from_tmax)(char *tmaxmsg, char **soapmsg, int *olen, char **service);

/* 외부 -> tmax로의 요청이 있을 경우 호출되는 callback 함수
   soapmsg [in] : 변환될 soapmsg
   soapmsglen [in] : soapmsg의 길이
   tmaxmsg [out] : 변환된 tmaxpmsg
   service [out] : 실제 서비스 이름
   */
int (*request_from_remote)(char *soapmsg, int soapmsglen, char **tmaxmsg, char **service);

/* tmax -> 외부로의 요청에 대한 응답이 있을 경우 호출되는 callback 함수
   soapmsg [in] : 변환될 soapmsg
   soapmsglen [in] : soapmsg의 길이
   tmaxmsg [out] : 변환된 tmaxpmsg
   olen [out] : tmaxmsg의 길이
   */
int (*reply_from_remote)(char *soapmsg, int soapmsglen, char **tmaxmsg, int *olen);

#endif

