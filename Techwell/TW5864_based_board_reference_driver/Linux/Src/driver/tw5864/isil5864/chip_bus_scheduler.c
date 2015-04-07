#include	<isil5864/isil_common.h>

static isil_root_bus_t  root_bus;
static type_bus_t       isil_host_bus;
static type_bus_t       isil_pci_bus;
static dpram_control_t  isil_dpram_controller;

static void register_node_init_complete(isil_register_node_t *node)
{
    if(node != NULL){
        init_completion(&node->wait_stop);
    }
}

static void register_node_wait_complete(isil_register_node_t *node)
{
    if(node != NULL){
        wait_for_completion(&node->wait_stop);
    }
}

static void register_node_complete_done(isil_register_node_t *node)
{
    if(node != NULL){
        complete_all(&node->wait_stop);
    }
}

static void register_node_init_wait_suspend_complete(isil_register_node_t *node)
{
    if(node != NULL){
        init_completion(&node->wait_suspend);
    }
}

static void register_node_wait_suspend_complete(isil_register_node_t *node)
{
    if(node != NULL){
        wait_for_completion(&node->wait_suspend);
    }
}

static void register_node_suspend_complete_done(isil_register_node_t *node)
{
    if(node != NULL){
        complete_all(&node->wait_suspend);
    }
}

static void register_node_init_wait_resume_complete(isil_register_node_t *node)
{
    if(node != NULL){
        init_completion(&node->wait_resume);
    }
}

static void register_node_wait_resume_complete(isil_register_node_t *node)
{
    if(node != NULL){
        wait_for_completion(&node->wait_resume);
    }
}

static void register_node_resume_complete_done(isil_register_node_t *node)
{
    if(node != NULL){
        complete_all(&node->wait_resume);
    }
}

static void register_node_init_all_complete(isil_register_node_t *node)
{
    if(node != NULL){
        init_completion(&node->wait_stop);
        init_completion(&node->wait_suspend);
        init_completion(&node->wait_resume);
    }
}

static void register_node_wait_all_complete(isil_register_node_t *node)
{
    if(node != NULL){
        wait_for_completion(&node->wait_stop);
        wait_for_completion(&node->wait_suspend);
        wait_for_completion(&node->wait_resume);
    }
}

static void register_node_all_complete_done(isil_register_node_t *node)
{
    if(node != NULL){
        complete_all(&node->wait_stop);
        complete_all(&node->wait_suspend);
        complete_all(&node->wait_resume);
    }
}

static struct isil_register_node_operation    isil_register_node_op = {
    .init_complete = register_node_init_complete,
    .wait_complete = register_node_wait_complete,
    .complete_done = register_node_complete_done,
    .init_wait_suspend_complete = register_node_init_wait_suspend_complete,
    .wait_suspend_complete = register_node_wait_suspend_complete,
    .suspend_complete_done = register_node_suspend_complete_done,
    .init_wait_resume_complete = register_node_init_wait_resume_complete,
    .wait_resume_complete = register_node_wait_resume_complete,
    .resume_complete_done = register_node_resume_complete_done,
    .init_all_complete = register_node_init_all_complete,
    .wait_all_complete = register_node_wait_all_complete,
    .all_complete_done = register_node_all_complete_done,
};

void    init_register_node(isil_register_node_t *node, void *priv, notify_register_node_priv notify_priv, match_regsiter_node_priv_id match_id)
{
    if(node != NULL){
        INIT_LIST_HEAD(&node->list);
        node->register_node_priv = priv;
        node->notify_priv = notify_priv;
        node->match_id = match_id;
        node->op = &isil_register_node_op;
        node->op->init_all_complete(node);
        node->op->all_complete_done(node);
    }
}

static void register_table_node_register_node_into_table(isil_register_table_t *register_table, isil_register_node_t *register_node)
{
    if((register_table!=NULL) && (register_node!=NULL)){
        unsigned long	flags;

        spin_lock_irqsave(&register_table->lock, flags);
        if(register_node->op != NULL){
            register_node->op->init_all_complete(register_node);
            register_node->op->suspend_complete_done(register_node);
            register_node->op->resume_complete_done(register_node);
        }
        list_add_tail(&register_node->list, &register_table->register_header);
        atomic_inc(&register_table->curr_register_entry_number);
        spin_unlock_irqrestore(&register_table->lock, flags);
    }
}

static void register_table_node_unregister_node_from_table(isil_register_table_t *register_table, isil_register_node_t *ptr_register_node)
{
    if((register_table!=NULL) && (ptr_register_node!=NULL)){
        isil_register_node_t	*register_node;
        struct list_head	*list;
        unsigned long	flags;

        spin_lock_irqsave(&register_table->lock, flags);
        if(atomic_read(&register_table->curr_register_entry_number)){
            list_for_each(list, &register_table->register_header){
                register_node = to_get_register_node(list);
                if(ptr_register_node == register_node){
                    atomic_dec(&register_table->curr_register_entry_number);
                    spin_unlock_irqrestore(&register_table->lock, flags);

                    if(ptr_register_node->notify_priv != NULL){
                        ptr_register_node->notify_priv(ptr_register_node, ptr_register_node->register_node_priv, NULL);
                    }
                    if(ptr_register_node->op != NULL){
                        ptr_register_node->op->wait_all_complete(ptr_register_node);
                    }

                    spin_lock_irqsave(&register_table->lock, flags);
                    list_del_init(&register_node->list);
                    if(atomic_read(&register_table->curr_register_entry_number) == 0){
                        INIT_LIST_HEAD(&register_table->register_header);
                    }
                    spin_unlock_irqrestore(&register_table->lock, flags);
                    return;
                }
            }
        }
        spin_unlock_irqrestore(&register_table->lock, flags);
    }
}

static void register_table_node_find_register_node_in_table(isil_register_table_t *register_table, isil_register_node_t **ptr_register_node, unsigned long arg_id)
{
    if((register_table!=NULL) && (ptr_register_node!=NULL)){
        isil_register_node_t	*register_node;
        struct list_head	*list;
        unsigned long	flags;

        *ptr_register_node = NULL;
        spin_lock_irqsave(&register_table->lock, flags);
        if(atomic_read(&register_table->curr_register_entry_number)){
            list_for_each(list, &register_table->register_header){
                register_node = to_get_register_node(list);
                if(register_node->match_id != NULL){
                    if(register_node->match_id(register_node, register_node->register_node_priv, arg_id) == ISIL_OK){
                        *ptr_register_node = register_node;
                        break;
                    }
                }
            }
        }
        spin_unlock_irqrestore(&register_table->lock, flags);
    }
}

/*get register_node from register_table's list with specified postion*/
static void register_table_node_get_register_node(isil_register_table_t *register_table, isil_register_node_t **ptr_register_node,unsigned long pos)
{
    if((register_table!=NULL) && (ptr_register_node!=NULL)){
        isil_register_node_t  *register_node;
        struct list_head    *list;
        unsigned long	    flags, nr,idx = 0;

        *ptr_register_node = NULL;
        spin_lock_irqsave(&register_table->lock, flags);
        nr = atomic_read(&register_table->curr_register_entry_number);
        if( nr || pos < nr ){
            list_for_each(list, &register_table->register_header){
                if(pos == idx++){	
                    register_node = to_get_register_node(list);
                    *ptr_register_node = register_node;
                    break;
                }
            }
        }
        spin_unlock_irqrestore(&register_table->lock, flags);
    }
}

static int  register_table_node_get_curr_register_node_number(isil_register_table_t *register_table)
{
    int	ret = 0;
    if(register_table != NULL){
        ret = atomic_read(&register_table->curr_register_entry_number);
    }
    return ret;
}

static struct register_table_node_operation	register_table_node_op = {
    .register_node_into_table = register_table_node_register_node_into_table,
    .unregister_node_from_table = register_table_node_unregister_node_from_table,
    .find_register_node_in_table = register_table_node_find_register_node_in_table,
    .get_node_from_table = register_table_node_get_register_node,
    .get_curr_register_node_number = register_table_node_get_curr_register_node_number,
};

void    init_register_table_node(isil_register_table_t *table)
{
    if(table != NULL){
        spin_lock_init(&table->lock);
        INIT_LIST_HEAD(&table->register_header);
        atomic_set(&table->curr_register_entry_number, 0);
        table->op = &register_table_node_op;
    }
}

void    get_root_bus(isil_root_bus_t **ptr_root_bus)
{
    *ptr_root_bus = &root_bus;
}

void    get_isil_host_bus(type_bus_t **ptr_type_bus)
{
    *ptr_type_bus = &isil_host_bus;
}

void    get_isil_pci_bus(type_bus_t **ptr_type_bus)
{
    *ptr_type_bus = &isil_pci_bus;
}

void    get_isil_dpram_controller(dpram_control_t **ptr_isil_dpram_controller)
{
    *ptr_isil_dpram_controller = &isil_dpram_controller;
}

void    init_isil_root_bus(void)
{
    init_register_table_node(&root_bus.root_register_table);
    init_type_bus(&isil_host_bus, ISIL_ED_HOST_BUS);
    init_type_bus(&isil_pci_bus, ISIL_ED_PCI_BUS);
    init_dpram_control(&isil_dpram_controller);
}

void    remove_isil_root_bus(void)
{
    isil_root_bus_t   *root_bus;
    type_bus_t      *isil_host_bus;
    type_bus_t      *isil_pci_bus;

    get_root_bus(&root_bus);
    get_isil_host_bus(&isil_host_bus);
    get_isil_pci_bus(&isil_pci_bus);
    isil_host_bus->register_node.op->all_complete_done(&isil_host_bus->register_node);
    root_bus->root_register_table.op->unregister_node_from_table(&root_bus->root_register_table, &isil_host_bus->register_node);
    isil_pci_bus->register_node.op->all_complete_done(&isil_pci_bus->register_node);
    root_bus->root_register_table.op->unregister_node_from_table(&root_bus->root_register_table, &isil_pci_bus->register_node);
}

static void notify_register_type_bus(isil_register_node_t *node, void *priv, isil_notify_msg *msg)
{
    if((node!=NULL) && (priv!=NULL)){
        isil_register_node_t    *register_node;
        struct list_head	*list;
        type_bus_t  *type_bus = (type_bus_t*)priv;
        isil_register_table_t   *type_bus_root_register_table = &type_bus->type_bus_root_register_table;
        unsigned long	flags;

        spin_lock_irqsave(&type_bus_root_register_table->lock, flags);
        if(atomic_read(&type_bus_root_register_table->curr_register_entry_number)){
            list_for_each(list, &type_bus_root_register_table->register_header){
                register_node = to_get_register_node(list);
                spin_unlock_irqrestore(&type_bus_root_register_table->lock, flags);

                if(register_node->notify_priv != NULL){
                    register_node->notify_priv(register_node, register_node->register_node_priv, NULL);
                }
                if(register_node->op != NULL){
                    register_node->op->wait_all_complete(register_node);
                }

                spin_lock_irqsave(&type_bus_root_register_table->lock, flags);
            }
        }
        spin_unlock_irqrestore(&type_bus_root_register_table->lock, flags);
    }
}

static int  match_regsiter_type_bus(isil_register_node_t *node, void *priv, unsigned long id)
{
    int	ret = ISIL_ERR;
    if((node!=NULL) && (priv!=NULL)){
        type_bus_t *bus = (type_bus_t*)priv;
        if(bus->bus_id == id){
            ret = ISIL_OK;
        }
    }
    return ret;
}

static void type_bus_init(type_bus_t *bus, int bus_id)
{
    if(bus != NULL){
        isil_root_bus_t	*rootBus;
        bus->bus_id = bus_id;
        atomic_set(&bus->device_id, 0);
        init_register_node(&bus->register_node, bus, notify_register_type_bus, match_regsiter_type_bus);
        init_register_table_node(&bus->type_bus_root_register_table);
        get_root_bus(&rootBus);
        if(rootBus->root_register_table.op != NULL){
            rootBus->root_register_table.op->register_node_into_table(&rootBus->root_register_table, &bus->register_node);
        }
    }
}

static void type_bus_register_chip_bus_into_type_bus_table(type_bus_t *bus, isil_chip_bus_t *chip_bus)
{
    if((bus!=NULL) && (chip_bus!=NULL)){
        isil_register_table_t	*type_bus_root_register_table;
        type_bus_root_register_table = &bus->type_bus_root_register_table;
        if(type_bus_root_register_table->op != NULL){
            type_bus_root_register_table->op->register_node_into_table(type_bus_root_register_table, &chip_bus->chip_register_node);
            atomic_inc(&bus->device_id);
        }
    }
}

static void type_bus_unregister_chip_bus_from_type_bus_table(type_bus_t *bus, isil_chip_bus_t *chip_bus)
{
    if((bus!=NULL) && (chip_bus!=NULL)){
        isil_register_table_t	*type_bus_root_register_table;

        type_bus_root_register_table = &bus->type_bus_root_register_table;
        if(type_bus_root_register_table->op != NULL){
            type_bus_root_register_table->op->unregister_node_from_table(type_bus_root_register_table, &chip_bus->chip_register_node);
            if(type_bus_root_register_table->op->get_curr_register_node_number(type_bus_root_register_table) <= 0){
                isil_register_node_t    *register_node;
                register_node = &bus->register_node;
                if(register_node->op != NULL){
                    register_node->op->all_complete_done(register_node);
                }            
            }
        }
    }
}

static void type_bus_find_chip_bus_in_type_bus_table(type_bus_t *bus, isil_chip_bus_t **chip_bus, int chip_id)
{
    if(bus != NULL){
        isil_register_table_t	*type_bus_root_register_table;
        isil_register_node_t 	*register_node;
        type_bus_root_register_table = &bus->type_bus_root_register_table;
        type_bus_root_register_table->op->find_register_node_in_table(type_bus_root_register_table, &register_node, chip_id);
        if(register_node != NULL){
            *chip_bus = to_get_chip_bus_with_chip_register_node(register_node);
        } else {
            *chip_bus = NULL;
        }
    }
}

static int type_bus_get_device_id(type_bus_t *bus)
{
    if(bus) {
        return atomic_read(&bus->device_id);
    }
    return -EINVAL;
}

static struct type_bus_operation    type_bus_op = {
    .init = type_bus_init,

    .register_chip_bus_into_type_bus_table = type_bus_register_chip_bus_into_type_bus_table,
    .unregister_chip_bus_from_type_bus_table = type_bus_unregister_chip_bus_from_type_bus_table,
    .find_chip_bus_in_type_bus_table = type_bus_find_chip_bus_in_type_bus_table,
    .get_device_id = type_bus_get_device_id,
};

void    init_type_bus(type_bus_t *bus, int bus_id)
{
    if(bus != NULL){
        bus->op = &type_bus_op;
        bus->op->init(bus, bus_id);
    }
}

void    remove_type_bus(type_bus_t *bus)
{
    if(bus != NULL){
        isil_root_bus_t	*rootBus;
        get_root_bus(&rootBus);
        rootBus->root_register_table.op->unregister_node_from_table(&rootBus->root_register_table, &bus->register_node);
    }
}

static void notify_register_chip_bus(isil_register_node_t *node, void *priv, isil_notify_msg *msg)
{
    if((node!=NULL) && (priv!=NULL)){

    }
}

static int  match_regsiter_chip_bus(isil_register_node_t *node, void *priv, unsigned long id)
{
    int	ret = ISIL_ERR;
    if((node!=NULL) && (priv!=NULL)){
        isil_chip_bus_t *bus = (isil_chip_bus_t*)priv;
        if(bus->chip_id == id){
            ret = ISIL_OK;
        }
    }
    return ret;
}

static void chip_bus_init(isil_chip_bus_t *chip_bus, int bus_id, int chip_id)
{
    if(chip_bus != NULL){
        type_bus_t  *type_bus;

        chip_bus->bus_id = bus_id;
        chip_bus->chip_id = chip_id;
        init_register_node(&chip_bus->chip_register_node, chip_bus, notify_register_chip_bus, match_regsiter_chip_bus);
        if(bus_id == ISIL_ED_HOST_BUS){
            get_isil_host_bus(&type_bus);
            if(type_bus->op != NULL){
                type_bus->op->register_chip_bus_into_type_bus_table(type_bus, chip_bus);
            }
        } else if(bus_id == ISIL_ED_PCI_BUS){
            get_isil_pci_bus(&type_bus);
            if(type_bus->op != NULL){
                type_bus->op->register_chip_bus_into_type_bus_table(type_bus, chip_bus);
            }
        } else {
            printk("%s.%d: no surpport bus\n", __FUNCTION__, __LINE__);
        }
    }
}

static struct chip_bus_operation    chip_bus_op = {
    .init = chip_bus_init,
};

void    init_chip_bus(isil_chip_bus_t *chip_bus, int bus_id, int chip_id)
{
    if(chip_bus != NULL){
        chip_bus->op = &chip_bus_op;
        chip_bus->op->init(chip_bus, bus_id, chip_id);
    }
}

void    remove_chip_bus(isil_chip_bus_t *chip_bus, int bus_id)
{
    type_bus_t              *type_bus;
    isil_register_table_t   *host_chip_table;
    isil_register_node_t    *register_node;
    if(chip_bus == NULL){
        return;
    }
    register_node = &chip_bus->chip_register_node;
    register_node->op->all_complete_done(register_node);
    if(bus_id == ISIL_ED_HOST_BUS){
        get_isil_host_bus(&type_bus);
        host_chip_table = &type_bus->type_bus_root_register_table;
        if(host_chip_table->op != NULL){
            host_chip_table->op->unregister_node_from_table(host_chip_table, register_node);
        }
    } else if(bus_id == ISIL_ED_PCI_BUS){
        get_isil_pci_bus(&type_bus);
        host_chip_table = &type_bus->type_bus_root_register_table;
        if(host_chip_table->op != NULL){
            host_chip_table->op->unregister_node_from_table(host_chip_table, register_node);
        }
    } else {
        printk("%s.%d: err bus_id %d\n", __FUNCTION__, __LINE__, bus_id);
    }
}

static void isil_ed_fsm_init(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        ed_fsm->last_state = ed_fsm->curr_state = ISIL_ED_UNREGISTER;
        ed_fsm->timeout_counter = 0;
        ed_fsm->context = NULL;
        spin_lock_init(&ed_fsm->lock);
        ed_fsm->transfer_pending.close_pending_flag = 0;
        ed_fsm->transfer_pending.timeout_pending_flag = 0;
        ed_fsm->transfer_pending.suspend_pending_flag = 0;
        ed_fsm->transfer_pending.resume_pending_flag = 0;
        ed_fsm->transfer_pending.open_pending_flag = 0;
        ed_fsm->transfer_pending.deliver_pending_flag = 0;
        ed_fsm->transfer_pending.need_sync = 0;
        ed_fsm->transfer_pending.reserve = 0;
        ed_fsm->robust_processing_pending.close_pending_flag = 0;
        ed_fsm->robust_processing_pending.timeout_pending_flag = 0;
        ed_fsm->robust_processing_pending.suspend_pending_flag = 0;
        ed_fsm->robust_processing_pending.resume_pending_flag = 0;
        ed_fsm->robust_processing_pending.open_pending_flag = 0;
        ed_fsm->robust_processing_pending.deliver_pending_flag = 0;
        ed_fsm->robust_processing_pending.need_sync = 0;
        ed_fsm->robust_processing_pending.reserve = 0;
        ed_fsm->table = NULL;
    }
}

static void isil_ed_fsm_reset(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->transfer_pending.close_pending_flag = 0;
        ed_fsm->transfer_pending.timeout_pending_flag = 0;
        ed_fsm->transfer_pending.suspend_pending_flag = 0;
        ed_fsm->transfer_pending.resume_pending_flag = 0;
        ed_fsm->transfer_pending.open_pending_flag = 0;
        ed_fsm->transfer_pending.deliver_pending_flag = 0;
        ed_fsm->transfer_pending.need_sync = 0;
        ed_fsm->transfer_pending.reserve = 0;
        ed_fsm->timeout_counter = 0;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_register_table(isil_ed_fsm_t *ed_fsm, fsm_state_transfer_matrix_table_t *fsm_table, void *context)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->context = context;
        ed_fsm->table = fsm_table;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_unregister_table(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->context = NULL;
        ed_fsm->table = NULL;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_update_need_sync_hook(isil_ed_fsm_t *ed_fsm, int en)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->transfer_pending.need_sync = en;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_update_enable_state_transfer(isil_ed_fsm_t *ed_fsm, int en)
{

}

static void isil_ed_fsm_update_enable_trigger_pending_event(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        if(ed_fsm->transfer_pending.open_pending_flag){
            ed_fsm->op->transfer(ed_fsm, ISIL_ED_OPEN_EVENT);
        }
        if(ed_fsm->transfer_pending.deliver_pending_flag){
            ed_fsm->op->transfer(ed_fsm, ISIL_ED_DELIVER_EVENT);
        }
        if(ed_fsm->transfer_pending.suspend_pending_flag){
            ed_fsm->op->transfer(ed_fsm, ISIL_ED_SUSPEND_EVENT);
        }
        if(ed_fsm->transfer_pending.resume_pending_flag){
            ed_fsm->op->transfer(ed_fsm, ISIL_ED_RESUME_EVENT);
        }
        if(ed_fsm->transfer_pending.timeout_pending_flag){
            ed_fsm->op->transfer(ed_fsm, ISIL_ED_TIMEOUT_EVENT);
        }
        if(ed_fsm->transfer_pending.close_pending_flag){
            ed_fsm->op->transfer(ed_fsm, ISIL_ED_CLOSE_EVENT);
        }
    }
}

static void isil_ed_fsm_change_state(isil_ed_fsm_t *ed_fsm, enum ISIL_ED_STATUS state)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->last_state = ed_fsm->curr_state;
        ed_fsm->curr_state = state;
        ed_fsm->timeout_counter = 0;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_change_state_for_robust(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->last_state = ed_fsm->curr_state;
        if((ed_fsm->last_state==ISIL_ED_RUNNING) || (ed_fsm->last_state==ISIL_ED_TRANSFERING)){
            ed_fsm->curr_state = ISIL_ED_DONE;
        }
        ed_fsm->timeout_counter = 0;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_req_fsm_event(isil_ed_fsm_t *ed_fsm, enum ISIL_ED_EVENT event)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        switch(event){
            case ISIL_ED_DELIVER_EVENT:
                ed_fsm->transfer_pending.deliver_pending_flag = 1;
                break;
            case ISIL_ED_TIMEOUT_EVENT:
                ed_fsm->transfer_pending.timeout_pending_flag = 1;
                break;
            case ISIL_ED_SUSPEND_EVENT:
                ed_fsm->transfer_pending.suspend_pending_flag = 1;
                break;
            case ISIL_ED_RESUME_EVENT:
                ed_fsm->transfer_pending.resume_pending_flag = 1;
                break;
            case ISIL_ED_OPEN_EVENT:
                ed_fsm->transfer_pending.open_pending_flag = 1;
                break;
            default:
            case ISIL_ED_CLOSE_EVENT:
                ed_fsm->transfer_pending.close_pending_flag = 1;
                break;
        }
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_ack_fsm_event(isil_ed_fsm_t *ed_fsm, enum ISIL_ED_EVENT event)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        switch(event){
            case ISIL_ED_DELIVER_EVENT:
                ed_fsm->transfer_pending.deliver_pending_flag = 0;
                break;
            case ISIL_ED_TIMEOUT_EVENT:
                ed_fsm->transfer_pending.timeout_pending_flag = 0;
                break;
            case ISIL_ED_SUSPEND_EVENT:
                ed_fsm->transfer_pending.suspend_pending_flag = 0;
                break;
            case ISIL_ED_RESUME_EVENT:
                ed_fsm->transfer_pending.resume_pending_flag = 0;
                break;
            case ISIL_ED_OPEN_EVENT:
                ed_fsm->transfer_pending.open_pending_flag = 0;
                break;
            default:
            case ISIL_ED_CLOSE_EVENT:
                ed_fsm->transfer_pending.close_pending_flag = 0;
                break;
        }
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static int  isil_ed_fsm_gen_fsm_event(isil_ed_fsm_t *ed_fsm, enum ISIL_ED_EVENT event, int can_transfer)
{
    int ret = ISIL_ERR;
    if(ed_fsm != NULL){
        ed_fsm->op->req_fsm_event(ed_fsm, event);
        if(can_transfer){
            ret = ed_fsm->op->transfer(ed_fsm, event);
        }
    }
    return ret;
}

static void isil_ed_fsm_save_event_for_robust_processing(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->robust_processing_pending.deliver_pending_flag = ed_fsm->transfer_pending.deliver_pending_flag;
        ed_fsm->robust_processing_pending.timeout_pending_flag = ed_fsm->transfer_pending.timeout_pending_flag;
        ed_fsm->robust_processing_pending.suspend_pending_flag = ed_fsm->transfer_pending.suspend_pending_flag;
        ed_fsm->robust_processing_pending.resume_pending_flag = ed_fsm->transfer_pending.resume_pending_flag;
        ed_fsm->robust_processing_pending.open_pending_flag = ed_fsm->transfer_pending.open_pending_flag;
        ed_fsm->robust_processing_pending.close_pending_flag = ed_fsm->transfer_pending.close_pending_flag;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static void isil_ed_fsm_restore_event_from_robust_processing(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->transfer_pending.deliver_pending_flag = ed_fsm->robust_processing_pending.deliver_pending_flag;
        ed_fsm->transfer_pending.timeout_pending_flag = ed_fsm->robust_processing_pending.timeout_pending_flag;
        ed_fsm->transfer_pending.suspend_pending_flag = ed_fsm->robust_processing_pending.suspend_pending_flag;
        ed_fsm->transfer_pending.resume_pending_flag = ed_fsm->robust_processing_pending.resume_pending_flag;
        ed_fsm->transfer_pending.open_pending_flag = ed_fsm->robust_processing_pending.open_pending_flag;
        ed_fsm->transfer_pending.close_pending_flag = ed_fsm->robust_processing_pending.close_pending_flag;
        ed_fsm->robust_processing_pending.deliver_pending_flag = 0;
        ed_fsm->robust_processing_pending.timeout_pending_flag = 0;
        ed_fsm->robust_processing_pending.suspend_pending_flag = 0;
        ed_fsm->robust_processing_pending.resume_pending_flag = 0;
        ed_fsm->robust_processing_pending.open_pending_flag = 0;
        ed_fsm->robust_processing_pending.close_pending_flag = 0;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static int isil_ed_fsm_transfer(isil_ed_fsm_t *ed_fsm, enum ISIL_ED_EVENT event)
{
    int ret = ISIL_ERR;

    if(ed_fsm != NULL){
        fsm_transfer_action action;
        fsm_transfer_action sync_hook;
        fsm_transfer_action sync_config_to_running_hook;
        fsm_transfer_action sync_running_to_config_hook;
        int last_state, curr_state;

        ed_fsm->op->ack_fsm_event(ed_fsm, event); 
        curr_state = ed_fsm->op->get_curr_state(ed_fsm);
        last_state = ed_fsm->op->get_last_state(ed_fsm);     
        if(ed_fsm->table != NULL){
            action = ed_fsm->table->action[curr_state][event];
            sync_hook = ed_fsm->table->sync_hook;
            sync_config_to_running_hook = ed_fsm->table->sync_config_to_running_hook;
            sync_running_to_config_hook = ed_fsm->table->sync_running_to_config_hook;
        } else {
            action = NULL;
            sync_hook = NULL;
            sync_config_to_running_hook = NULL;
            sync_running_to_config_hook = NULL;
        }

        if(action != NULL){
            ret = action(ed_fsm, ed_fsm->context);
            curr_state = ed_fsm->op->get_curr_state(ed_fsm);
            last_state = ed_fsm->op->get_last_state(ed_fsm);     
            if(ed_fsm->transfer_pending.need_sync){
                if((sync_hook!=NULL) && (curr_state==ISIL_ED_IDLE) && ((last_state==ISIL_ED_SUSPEND)||(last_state==ISIL_ED_DONE))){
                    ed_fsm->op->update_need_sync_hook(ed_fsm, 0);
                    ret = sync_hook(ed_fsm, ed_fsm->context);       
                }
            }
        }

        if(sync_config_to_running_hook != NULL){
            if((curr_state==ISIL_ED_IDLE) && ((last_state==ISIL_ED_SUSPEND)||(last_state==ISIL_ED_DONE))){
                ret = sync_config_to_running_hook(ed_fsm, ed_fsm->context);
            }
        }

        if(sync_running_to_config_hook != NULL){
            if((curr_state==ISIL_ED_UNREGISTER) && (last_state!=ISIL_ED_UNREGISTER)){
                ret = sync_running_to_config_hook(ed_fsm, ed_fsm->context);
            }
        }
    }
    return ret;
}

static int isil_ed_fsm_get_curr_state(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        return ed_fsm->curr_state;
    } else {
        return ISIL_ED_UNREGISTER;
    }
}

static int isil_ed_fsm_get_last_state(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        return ed_fsm->last_state;
    } else {
        return ISIL_ED_UNREGISTER;
    }
}

static void isil_ed_fsm_update_state_timeout_counter(isil_ed_fsm_t *ed_fsm)
{
    /*if(ed_fsm != NULL){
        unsigned long   flags;
        int need_gen_timeout_event, limit_timeout_counter;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->timeout_counter++;
        switch(ed_fsm->op->get_curr_state(ed_fsm)){
            case ISIL_ED_IDLE:
                limit_timeout_counter = ISIL_IDLE_CAN_DURATION_MAX_COUNTER;
                break;
            case ISIL_ED_STANDBY:
                limit_timeout_counter = ISIL_STANDBY_CAN_DURATION_MAX_COUNTER;
                break;
            case ISIL_ED_RUNNING:
                limit_timeout_counter = ISIL_RUNNING_CAN_DURATION_MAX_COUNTER;
                break;
            case ISIL_ED_TRANSFERING:
                limit_timeout_counter = ISIL_TRANSFERING_CAN_DURATION_MAX_COUNTER;
                break;
            case ISIL_ED_DONE:
                limit_timeout_counter = ISIL_DONE_CAN_DURATION_MAX_COUNTER;
                break;
            default:
            case ISIL_ED_UNREGISTER:
            case ISIL_ED_SUSPEND:
                limit_timeout_counter = ISIL_CAN_DURATION_MAX_COUNTER;
                ed_fsm->timeout_counter = 0;
                break;
        }
        if(ed_fsm->timeout_counter >= limit_timeout_counter){
            need_gen_timeout_event = 1;
            ed_fsm->timeout_counter = 0;
            ISIL_DBG(ISIL_DBG_ERR, "timeout_counter = %d\n", limit_timeout_counter);
        } else {
            need_gen_timeout_event = 0; 
        }
        spin_unlock_irqrestore(&ed_fsm->lock, flags);

        if(need_gen_timeout_event){
            ed_tcb_t *ed_tcb;
            ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
            driver_gen_timeout_event(ed_tcb, 1);
        }
    }*/
}

static void isil_ed_fsm_feed_state_watchdog(isil_ed_fsm_t *ed_fsm)
{
    if(ed_fsm != NULL){
        unsigned long   flags;
        spin_lock_irqsave(&ed_fsm->lock, flags);
        ed_fsm->timeout_counter = 0;
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

static struct isil_ed_fsm_operation isil_ed_fsm_op ={
    .init = isil_ed_fsm_init,
    .reset = isil_ed_fsm_reset,
    .register_table = isil_ed_fsm_register_table,
    .unregister_table = isil_ed_fsm_unregister_table,
    .update_need_sync_hook = isil_ed_fsm_update_need_sync_hook,
    .update_enable_state_transfer = isil_ed_fsm_update_enable_state_transfer,
    .update_enable_trigger_pending_event = isil_ed_fsm_update_enable_trigger_pending_event,
    .change_state = isil_ed_fsm_change_state,
    .change_state_for_robust = isil_ed_fsm_change_state_for_robust,
    .req_fsm_event = isil_ed_fsm_req_fsm_event,
    .ack_fsm_event = isil_ed_fsm_ack_fsm_event,
    .gen_fsm_event = isil_ed_fsm_gen_fsm_event,
    .save_event_for_robust_processing = isil_ed_fsm_save_event_for_robust_processing,
    .restore_event_from_robust_processing = isil_ed_fsm_restore_event_from_robust_processing,
    .transfer = isil_ed_fsm_transfer,
    .get_curr_state = isil_ed_fsm_get_curr_state,
    .get_last_state = isil_ed_fsm_get_last_state,
    .update_state_timeout_counter = isil_ed_fsm_update_state_timeout_counter,
    .feed_state_watchdog = isil_ed_fsm_feed_state_watchdog,
 };

static void init_ed_fsm(isil_ed_fsm_t *ed_fsm, fsm_state_transfer_matrix_table_t *fsm_table, void *context)
{
    if(ed_fsm != NULL){
        ed_fsm->op = &isil_ed_fsm_op;
        ed_fsm->op->init(ed_fsm);
        ed_fsm->op->register_table(ed_fsm, fsm_table, context);
    }
}

int  no_op(isil_ed_fsm_t *ed_fsm, void *context)
{
    return ISIL_OK;
}

int have_any_pending_event(ed_tcb_t *ed_tcb)
{
    int ret = 0;
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        unsigned long   flags;

        spin_lock_irqsave(&ed_fsm->lock, flags);
        if(ed_fsm->transfer_pending.open_pending_flag){
            ret = 1;
        }
        if(ed_fsm->transfer_pending.suspend_pending_flag){
            ret = 1;
        }
        if(ed_fsm->transfer_pending.resume_pending_flag){
            ret = 1;
        }
        if(ed_fsm->transfer_pending.timeout_pending_flag){
            ret = 1;
        }
        if(ed_fsm->transfer_pending.close_pending_flag){
            ret = 1;
        }
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
    return ret;
}

void    driver_trigger_pending_event(ed_tcb_t *ed_tcb)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->update_enable_trigger_pending_event(ed_fsm);
    }
}

void    driver_gen_close_event(ed_tcb_t *ed_tcb, int can_transfer)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->gen_fsm_event(ed_fsm, ISIL_ED_CLOSE_EVENT, can_transfer);
    }
}

void    driver_gen_timeout_event(ed_tcb_t *ed_tcb, int can_transfer)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->gen_fsm_event(ed_fsm, ISIL_ED_TIMEOUT_EVENT, can_transfer);
    }
}

void    driver_gen_suspend_event(ed_tcb_t *ed_tcb, int can_transfer)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->gen_fsm_event(ed_fsm, ISIL_ED_SUSPEND_EVENT, can_transfer);
    }
}

void    driver_gen_resume_event(ed_tcb_t *ed_tcb, int can_transfer)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->gen_fsm_event(ed_fsm, ISIL_ED_RESUME_EVENT, can_transfer);
    }
}

void    driver_gen_open_event(ed_tcb_t *ed_tcb, int can_transfer)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->gen_fsm_event(ed_fsm, ISIL_ED_OPEN_EVENT, can_transfer);
    }
}

void    driver_gen_deliver_event(ed_tcb_t *ed_tcb, int can_transfer)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t *ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->gen_fsm_event(ed_fsm, ISIL_ED_DELIVER_EVENT, can_transfer);
    }
}

void    direct_driver_config_to_running(ed_tcb_t *ed_tcb)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t       *ed_fsm = &ed_tcb->ed_fsm;
        fsm_transfer_action sync_config_to_running_hook;
        unsigned long       flags;

        spin_lock_irqsave(&ed_fsm->lock, flags);
        sync_config_to_running_hook = ed_fsm->table->sync_config_to_running_hook;
        if(sync_config_to_running_hook != NULL){
            sync_config_to_running_hook(ed_fsm, ed_fsm->context);
        }
        spin_unlock_irqrestore(&ed_fsm->lock, flags);
    }
}

void    driver_sync_config_to_running(ed_tcb_t *ed_tcb)
{
    if(ed_tcb != NULL){
        isil_ed_fsm_t   *ed_fsm = &ed_tcb->ed_fsm;

        if(ed_fsm->op->get_curr_state(ed_fsm) == ISIL_ED_SUSPEND){
            direct_driver_config_to_running(ed_tcb);
        }
    }
}

static void init_control_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id, int logic_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_CONTROL;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = (logic_chan_id)&0x3ff;
    }
}

static void	init_video_encode_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id, int logic_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_VIDEO_ENCODE_IN;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = (logic_chan_id)&0x3ff;
    }
}

static void	init_video_encode_preview_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id, int logic_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_VIDEO_PREVIEW_IN;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = (logic_chan_id)&0x3ff;
    }
}

static void	init_video_encode_osd_out_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id, int logic_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_VIDEO_ENCODE_OSD_OUT;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = (logic_chan_id)&0x3ff;
    }
}

static void	init_video_decode_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id, int logic_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_VIDEO_DECODE_OUT;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = (logic_chan_id)&0x3ff;
    }
}

static void	init_video_decode_preview_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id, int logic_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_VIDEO_PREVIEW_OUT;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = (logic_chan_id)&0x3ff;
    }
}

static void	init_video_decode_osg_out_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id, int logic_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_VIDEO_DECODE_OSG_OUT;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = (logic_chan_id)&0x3ff;
    }
}

static void	init_audio_encode_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_AUDIO_ENCODE_IN;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = 0;
    }
}

static void	init_audio_decode_ed(ed_id *id, int bus_id, int chip_id, int group_chan_id)
{
    if(id != NULL){
        id->bus_id = (bus_id)&0xf;
        id->chip_id = (chip_id)&0xf;
        id->type_id = ISIL_ED_AUDIO_DECODE_OUT;
        id->group_chan_id = (group_chan_id)&0x3ff;
        id->logic_chan_id = 0;
    }
}

static void	init_invalid_ed(ed_id *id)
{
    if(id != NULL){
        id->bus_id = INVALID_ISIL_ED_BSU_ID;
        id->chip_id = 0;
        id->type_id = INVALID_ISIL_ED_TYPE_ID;
        id->group_chan_id = 0;
        id->logic_chan_id = 0;
    }
}

static void	ed_tcb_init(ed_tcb_t *ed_tcb, int bus_id, int chip_id, int type_id, int group_chan_id, int logic_chan_id)
{
    if(ed_tcb != NULL){
        switch(type_id){
            case ISIL_ED_CONTROL:
                init_control_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id, logic_chan_id);
                break;
            case ISIL_ED_VIDEO_ENCODE_IN:
                init_video_encode_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id, logic_chan_id);
                break;
            case ISIL_ED_VIDEO_PREVIEW_IN:
                init_video_encode_preview_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id, logic_chan_id);
                break;
            case ISIL_ED_VIDEO_ENCODE_OSD_OUT:
                init_video_encode_osd_out_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id, logic_chan_id);
                break;
            case ISIL_ED_VIDEO_DECODE_OUT:
                init_video_decode_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id, logic_chan_id);
                break;
            case ISIL_ED_VIDEO_PREVIEW_OUT:
                init_video_decode_preview_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id, logic_chan_id);
                break;
            case ISIL_ED_VIDEO_DECODE_OSG_OUT:
                init_video_decode_osg_out_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id, logic_chan_id);
                break;
            case ISIL_ED_AUDIO_ENCODE_IN:
                init_audio_encode_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id);
                break;
            case ISIL_ED_AUDIO_DECODE_OUT:
                init_audio_decode_ed(&ed_tcb->id, bus_id, chip_id, group_chan_id);
                break;
            default:
            case INVALID_ISIL_ED_TYPE_ID:
                init_invalid_ed(&ed_tcb->id);
                break;
        }
        atomic_set(&ed_tcb->state, ISIL_ED_UNREGISTER);
    }
}

void    init_endpoint_tcb(ed_tcb_t *ed_tcb, int bus_id, int chip_id, int type_id, int group_chan_id, int logic_chan_id, void *priv, notify_register_node_priv notify_priv, match_regsiter_node_priv_id match_id, fsm_state_transfer_matrix_table_t *fsm_table)
{
    if(ed_tcb != NULL){
        ed_tcb_init(ed_tcb, bus_id, chip_id, type_id, group_chan_id, logic_chan_id);
        init_register_node(&ed_tcb->ed, priv, notify_priv, match_id);
        init_ed_fsm(&ed_tcb->ed_fsm, fsm_table, priv);
    }
}

static void robust_processing_control_init(robust_processing_control_t *robust)
{
    if(robust != NULL){
        atomic_set(&robust->robust_processing_flag, 0);
        init_completion(&robust->robust_processing_done);
    }
}

static void robust_processing_control_start_robust_processing(robust_processing_control_t *robust)
{
    if(robust != NULL){
        atomic_set(&robust->robust_processing_flag, 1);
        init_completion(&robust->robust_processing_done);
    }
}

static void robust_processing_control_robust_processing_done(robust_processing_control_t *robust)
{
    if(robust != NULL){
        complete_all(&robust->robust_processing_done);
        atomic_set(&robust->robust_processing_flag, 0);
    }
}

static void robust_processing_control_wait_robust_processing_done(robust_processing_control_t *robust)
{
    if(robust != NULL){
        if(atomic_read(&robust->robust_processing_flag) == 1){
            wait_for_completion(&robust->robust_processing_done);
        }
    }
}

static int  robust_processing_control_is_in_robust_processing(robust_processing_control_t *robust)
{
    return atomic_read(&robust->robust_processing_flag);
}

static struct robust_processing_control_operation   robust_processing_control_op ={
    .init = robust_processing_control_init,
    .start_robust_processing = robust_processing_control_start_robust_processing,
    .robust_processing_done = robust_processing_control_robust_processing_done,
    .wait_robust_processing_done = robust_processing_control_wait_robust_processing_done,
    .is_in_robust_processing = robust_processing_control_is_in_robust_processing,
};

void    init_robust_processing_control(robust_processing_control_t *robust)
{
    if(robust != NULL){
        robust->op = &robust_processing_control_op;
        robust->op->init(robust);
    }
}


