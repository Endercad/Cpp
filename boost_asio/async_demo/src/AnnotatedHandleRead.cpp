/**
 * TCP消息解析器 - 解决粘包/拆包问题
 * 
 * 协议格式：[头部(HEAD_LENGTH字节)] + [消息体(变长)]
 * - 头部：固定长度，存储后续消息体的长度(short类型)
 * - 消息体：变长，实际的消息内容
 * 
 * 核心思想：
 * 1. 使用状态机处理不同的接收状态
 * 2. 支持跨多次接收重组完整消息
 * 3. 一次接收可能包含多个消息，需要循环处理
 */

void CSession::HandleRead(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<CSession> shared_self) {
    if (!error) {
        /*
         * 🎯 核心变量说明：
         * - copy_len: 本次已处理的字节数指针
         * - bytes_transferred: 本次接收到的总字节数
         * - _b_head_parse: 状态标志，false=正在解析头部，true=正在解析消息体
         * - _recv_head_node: 头部数据缓冲区，保存跨接收的头部片段
         * - _recv_msg_node: 消息体缓冲区，保存跨接收的消息体片段
         */
        
        int copy_len = 0;  // 当前已处理的字节数
        
        /*
         * 🔄 主处理循环：
         * 为什么用while？因为一次接收可能包含：
         * - 多个完整消息
         * - 一个消息的部分 + 另一个消息的部分
         * - 上个消息的尾部 + 若干完整消息 + 下个消息的头部
         */
        while (bytes_transferred > 0) {
            
            /*
             * 🏗️ 状态机分支1：解析头部阶段
             * 条件：_b_head_parse == false（还没完成头部解析）
             */
            if (!_b_head_parse) {
                
                /*
                 * 📦 情况1.1：头部数据不足
                 * 场景：本次接收的数据 + 之前积累的头部数据 < HEAD_LENGTH
                 * 例如：HEAD_LENGTH=4，之前收到1字节，本次收到2字节，总共3字节 < 4字节
                 */
                if (bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH) {
                    // 将本次接收的所有数据都追加到头部缓冲区
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;  // 更新头部已接收长度
                    
                    // 清空接收缓冲区，准备下次接收
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    return;  // 等待更多数据来完成头部
                }
                
                /*
                 * 📦 情况1.2：头部数据足够（可能还包含消息体数据）
                 * 场景：本次接收的数据足以完成头部解析
                 */
                
                // 计算完成头部还需要多少字节
                int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
                
                // 将头部的剩余部分拷贝完成
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                
                // 更新处理进度
                copy_len += head_remain;              // 移动已处理指针
                bytes_transferred -= head_remain;     // 减少剩余未处理字节数
                
                /*
                 * 🔍 解析头部：提取消息体长度
                 * 头部存储的是后续消息体的长度信息
                 */
                short data_len = 0;
                memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);
                cout << "data_len is " << data_len << endl;
                
                // 🛡️ 安全检查：防止恶意数据导致内存溢出
                if (data_len > MAX_LENGTH) {
                    std::cout << "invalid data length is " << data_len << endl;
                    _server->ClearSession(_uuid);
                    return;
                }
                
                // 根据头部信息创建消息体缓冲区
                _recv_msg_node = make_shared<MsgNode>(data_len);
                
                /*
                 * 📦 子情况1.2.1：消息体数据不足
                 * 场景：除去头部后，剩余数据不足以构成完整消息体
                 * 例如：消息体需要100字节，但除去头部后只剩50字节
                 */
                if (bytes_transferred < data_len) {
                    // 将剩余的所有数据都拷贝到消息体缓冲区
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    
                    // 清空接收缓冲区，准备下次接收
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    
                    // 🎯 关键：标记头部已解析完成，下次进入消息体解析状态
                    _b_head_parse = true;
                    return;  // 等待更多数据来完成消息体
                }
                
                /*
                 * 📦 子情况1.2.2：消息体数据足够完整
                 * 场景：除去头部后，剩余数据足以构成完整消息体（可能还有多余）
                 */
                
                // 拷贝完整的消息体数据
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, data_len);
                _recv_msg_node->_cur_len += data_len;
                
                // 更新处理进度
                copy_len += data_len;
                bytes_transferred -= data_len;
                
                // 添加字符串结束符
                _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
                cout << "receive data is " << _recv_msg_node->_data << endl;
                
                /*
                 * 🎉 消息处理：一个完整消息接收完成
                 * 这里调用Send进行回显（Echo服务器功能）
                 */
                Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
                
                /*
                 * 🔄 重置状态，准备处理下一个消息
                 * 因为剩余数据中可能还包含其他消息
                 */
                _b_head_parse = false;      // 重置为头部解析状态
                _recv_head_node->Clear();   // 清空头部缓冲区
                
                // 如果没有剩余数据，继续等待接收
                if (bytes_transferred <= 0) {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    return;
                }
                
                // 如果还有剩余数据，继续while循环处理
                continue;
            }
            
            /*
             * 🏗️ 状态机分支2：解析消息体阶段
             * 条件：_b_head_parse == true（头部已解析完成，正在接收消息体）
             * 场景：上次已经解析完头部，但消息体没有接收完整
             */
            
            // 计算消息体还需要多少字节才能完整
            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            
            /*
             * 📦 情况2.1：消息体仍然不足
             * 场景：本次接收的数据仍然不足以完成当前消息体
             * 例如：还需要100字节，但本次只收到50字节
             */
            if (bytes_transferred < remain_msg) {
                // 将本次接收的所有数据都追加到消息体缓冲区
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                _recv_msg_node->_cur_len += bytes_transferred;
                
                // 清空接收缓冲区，继续等待
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
                    std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                return;  // 等待更多数据来完成消息体
            }
            
            /*
             * 📦 情况2.2：消息体足够完整
             * 场景：本次接收的数据足以完成当前消息体（可能还有多余）
             */
            
            // 拷贝消息体的剩余部分
            memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            
            // 更新处理进度
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            
            // 添加字符串结束符
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            cout << "receive data is " << _recv_msg_node->_data << endl;
            
            /*
             * 🎉 消息处理：一个完整消息接收完成
             */
            Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
            
            /*
             * 🔄 重置状态，准备处理下一个消息
             */
            _b_head_parse = false;      // 重置为头部解析状态
            _recv_head_node->Clear();   // 清空头部缓冲区
            
            // 如果没有剩余数据，继续等待接收
            if (bytes_transferred <= 0) {
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                return;
            }
            
            // 如果还有剩余数据，继续while循环处理
            continue;
        }
    }
    else {
        /*
         * ❌ 错误处理：网络错误或连接断开
         */
        std::cout << "handle read failed, error is " << error.what() << endl;
        Close();
        _server->ClearSession(_uuid);
    }
}

/*
 * 📚 总结：这个函数解决的核心问题
 * 
 * 1. TCP粘包问题：
 *    - 发送方：Send("Hello") + Send("World")
 *    - 接收方：可能收到"HelloWorld"
 *    - 解决：通过头部长度信息正确分割
 * 
 * 2. TCP拆包问题：
 *    - 发送方：Send("一个很长的消息...")
 *    - 接收方：可能分多次才收完整
 *    - 解决：通过状态保持，跨多次接收重组
 * 
 * 3. 混合情况：
 *    - 一次接收：[上个消息尾部] + [完整消息1] + [完整消息2] + [下个消息头部]
 *    - 解决：通过状态机和while循环，正确解析所有情况
 * 
 * 4. 关键设计：
 *    - 状态机：_b_head_parse标记当前解析阶段
 *    - 缓冲管理：_recv_head_node和_recv_msg_node保存跨接收数据
 *    - 循环处理：while循环处理一次接收中的多个消息
 *    - 边界检查：防止恶意数据导致内存问题
 */ 