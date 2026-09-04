/**
 * @file test_mqtt_discovery.cpp
 * @brief Unit tests for the MqttDiscovery driver.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-09-02
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "network/MqttDiscovery.h"
#include "../mocks/MockMdns.h"

using namespace vigilo;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::SetArrayArgument;
using ::testing::DoAll;
using ::testing::_;

class MqttDiscoveryTest : public ::testing::Test {
protected:
    MockMdns      mdns;
    MqttDiscovery discovery{mdns};
    char          broker[MqttDiscovery::BROKER_ADDRESS_CAPACITY];
};

TEST_F(MqttDiscoveryTest, DiscoversBrokerWhenServiceFound) {
    static const char resolved[] = "192.168.1.20";

    EXPECT_CALL(mdns, begin(StrEq("vigilo-01"))).WillOnce(Return(true));
    EXPECT_CALL(mdns, queryService(StrEq("mqtt"), StrEq("tcp"), _, sizeof(broker)))
        .WillOnce(DoAll(SetArrayArgument<2>(resolved, resolved + sizeof(resolved)),
                        Return(IMdns::QueryResult::Found)));

    EXPECT_TRUE(discovery.discover("vigilo-01", "mqtt", "tcp", broker, sizeof(broker), "10.0.0.1"));
    EXPECT_STREQ(broker, resolved);
}

TEST_F(MqttDiscoveryTest, FallsBackWhenServiceNotFound) {
    EXPECT_CALL(mdns, begin(_)).WillOnce(Return(true));
    EXPECT_CALL(mdns, queryService(_, _, _, _)).WillOnce(Return(IMdns::QueryResult::NotFound));

    EXPECT_FALSE(discovery.discover("vigilo-01", "mqtt", "tcp", broker, sizeof(broker), "10.0.0.1"));
    EXPECT_STREQ(broker, "10.0.0.1");
}

TEST_F(MqttDiscoveryTest, FallsBackWhenMdnsBeginFails) {
    EXPECT_CALL(mdns, begin(_)).WillOnce(Return(false));
    EXPECT_CALL(mdns, queryService(_, _, _, _)).Times(0);

    EXPECT_FALSE(discovery.discover("vigilo-01", "mqtt", "tcp", broker, sizeof(broker), "10.0.0.1"));
    EXPECT_STREQ(broker, "10.0.0.1");
}