static int idl_ipname_family = 0;
static dns_ipaddr_t idl_ipname_ipaddr;

static void idl_dns_pname_hash_get(struct idl_param_st *p, struct idl_buf_st *output, void *userdat)
{
    sf_ht_t *table = dp_dns_get_ipname_table();
    dp_dns_ipname_entry_t* cache;
    vsys_id_t vsys_id = SF_ROOT_VSYS_ID;
    int max_ipname = 10;

    if (!idl_ipname_family){
        for (sf_uint32_t id = 0; (id <= table->et_size)&&(max_ipname > 0); id++) {
            cache = (dp_dns_ipname_entry_t *)sf_ht_get_entity_by_id(table, id);
            if (cache == NULL || !sf_ht_is_entity_node_be_use(&(cache->maintain_item))) {
                continue;
            }

            idl_buf_printf(output, 256, "ip %s:\n", dp_dns_ipaddr_tostr(cache->data.family, &cache->data.ipaddr));
            for (int position = 0; position < cache->data.nr_names; position++) {
                idl_buf_printf(output, 256, "        %s\n", cache->data.names[position]);
            }

            max_ipname--;
        }

        idl_buf_printf(output, 256, "Query the specified ip:\n    \"192.168.1.1\"\n    \"2001::1\"\n");
    } else {
        cache = dp_dns_ipname_ht_lookup(table, idl_ipname_family, &idl_ipname_ipaddr, vsys_id);
        if (cache != NULL && sf_ht_is_entity_node_be_use(&(cache->maintain_item))) {
            idl_buf_printf(output, 256, "ip %s:\n", dp_dns_ipaddr_tostr(cache->data.family, &cache->data.ipaddr));
            for (int position = 0; position < cache->data.nr_names; position++) {
                idl_buf_printf(output, 256, "        %s\n", cache->data.names[position]);
            }
        } else {
            idl_buf_printf(output, 256, "ip %s:no domain\n", dp_dns_ipaddr_tostr(idl_ipname_family, &idl_ipname_ipaddr));
        }
    }

    idl_ipname_family = 0;
    memset(&idl_ipname_ipaddr, 0, sizeof(dns_ipaddr_t));
    return;
}

static int idl_dns_pname_hash_set(struct idl_param_st *p, const char *output, void *userdata)
{
    char ipaddr4or6[INET6_ADDRSTRLEN + 1] = {'\0'};
    idl_ipname_family = 0;
    memset(&idl_ipname_ipaddr, 0, sizeof(dns_ipaddr_t));
    if (strlen(output) > INET6_ADDRSTRLEN || sscanf(output, "%s\n", ipaddr4or6) == 0) {
        log_debug("dp dns", "ip host", "[error]output %s\n", output);
        return 2;
    }

    if (dns_parse_inet_addr_str((sf_uint8_t *)ipaddr4or6, &idl_ipname_family, &idl_ipname_ipaddr) != 0) {
        log_debug("dp dns", "ip host", "[error] no ipaddr4or6 %s\n", ipaddr4or6);
        return 1;
    }

    return 0;
}

IDL_DBG_DATA_ENTRY(idl_dns_ipname_hash, "dns.ipname", NULL, NULL, 
    idl_dns_pname_hash_get, idl_dns_pname_hash_set, NULL, "查看本机dns对应ip查域名缓存");



int main(int argc, char const *argv[])
{
    
    return 0;
}

