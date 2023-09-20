#pragma once

namespace zk {
    enum OperatorType{
        EXISTS, 
        NOT_EXISTS, 
        EQUALS, 
        NOT_EQUALS,
        IN,
        NOT_IN,
        GREATER_THAN,
        GREATER_THAN_EQUALS,
        LESS_THAN_EQUALS,
        LESS_THAN,
    };
}