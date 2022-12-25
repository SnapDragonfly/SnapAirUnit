import Vue from 'vue'
import Router from 'vue-router'
import Home from './views/Home.vue'
import Wireless from './views/Wireless.vue'
import RcChannel from './views/RcChannel.vue'

Vue.use(Router)

export default new Router({
  mode: 'history',
  base: process.env.BASE_URL,
  routes: [
    {
      path: '/',
      name: 'home',
      component: Home
    },
    {
      path: '/wireless',
      name: 'wireless',
      component: Wireless
    },
    {
      path: '/rc',
      name: 'rc',
      component: RcChannel
    }
  ]
})
