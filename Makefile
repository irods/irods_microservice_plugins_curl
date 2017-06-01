MS_PLUGIN_CURL_CORE = microservices/core
MSI_CURL_GET_OBJ	= microservices/msiCurlGetObj
MSI_CURL_GET_STR	= microservices/msiCurlGetStr
MSI_CURL_POST		= microservices/msiCurlPost
MSI_CURL_PUT		= microservices/msiCurlPut

MAKEFLAGS += --no-print-directory

LIBS = $(MS_PLUGIN_CURL_CORE) $(MSI_CURL_GET_OBJ) $(MSI_CURL_GET_STR) $(MSI_CURL_POST) $(MSI_CURL_PUT)

.PHONY: all $(LIBS) clean
all: $(LIBS)

$(LIBS):
	@$(MAKE) -C $@;

$(MSI_CURL_GET_OBJ): $(MS_PLUGIN_CURL_CORE)

$(MSI_CURL_GET_STR): $(MS_PLUGIN_CURL_CORE)

$(MSI_CURL_POST): $(MS_PLUGIN_CURL_CORE)

$(MSI_CURL_PUT): $(MS_PLUGIN_CURL_CORE)

clean:
	@$(MAKE) -C $(MS_PLUGIN_CURL_CORE) clean;
	@$(MAKE) -C $(MSI_CURL_GET_OBJ) clean;
	@$(MAKE) -C $(MSI_CURL_GET_STR) clean;
	@$(MAKE) -C $(MSI_CURL_POST) clean;
	@$(MAKE) -C $(MSI_CURL_PUT) clean;
