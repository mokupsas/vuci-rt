import Vue from 'vue'
import Router from 'vue-router'
import { session } from '@/plugins/session'
import { rpc } from '@/plugins/rpc'
import axios from 'axios'

Vue.use(Router)

const router = new Router({
  routes: [{
    path: '/wizard',
    component: resolve => {
      axios.get(`/views/vuci-app-wizard.js?_t=${new Date().getTime()}`).then(r => {
        // eslint-disable-next-line no-eval
        return resolve(eval(r.data))
      }).catch(() => {
        return resolve(require('@/views/Wizard.vue'))
      })
    }
  },
  {
    path: '/login',
    component: resolve => {
      axios.get(`/views/vuci-app-login.js?_t=${new Date().getTime()}`).then(r => {
        // eslint-disable-next-line no-eval
        return resolve(eval(r.data))
      }).catch(() => {
        return resolve(require('@/views/Login.vue'))
      })
    }
  },
  {
    path: '/',
    redirect: '/home',
    component: () =>
      import('@/components/VuciLayout'),
    children: [{
      path: 'home',
      component: resolve => {
        axios.get(`/views/vuci-app-home.js?_t=${new Date().getTime()}`).then(r => {
          // eslint-disable-next-line no-eval
          return resolve(eval(r.data))
        }).catch(() => {
          return resolve(require('@/views/Home.vue'))
        })
      },
      meta: {
        title: 'Home'
      }
    }]
  },
  {
    path: '*',
    component: () =>
      import('@/components/404.vue')
  }
  ]
})

function beforeEach (to, next, alive) {
  if (to.path !== '/login') {
    if (alive) {
      next()
    } else {
      next('/login')
    }
  } else {
    next()
  }
}

function firstLogin () {
  return new Promise(resolve => {
    rpc.call('ui', 'first_login').then(r => {
      resolve(r.first)
    })
  })
}

router.beforeEach((to, from, next) => {
  firstLogin().then(first => {
    if (first) {
      if (to.path !== '/wizard') { next('/wizard') } else { next() }
    } else {
      if (to.path === '/wizard') {
        next('/')
        return
      }

      session.isAlive().then((alive) => {
        if (alive) session.startHeartbeat()
        else session.logout()

        beforeEach(to, next, alive)
      })
    }
  })
})

export default router
