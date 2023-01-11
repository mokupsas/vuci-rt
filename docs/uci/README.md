# Guide

Vuci encapsulates some Vue components for uci, which is convenient for developers to call.

``` vue
<template>
  <vuci-form uci-config="test">
    <vuci-named-section title="Test" name="main" v-slot="{ s }">
      <vuci-form-item-input label="Name" name="name" :uci-section="s"/>
    </vuci-named-section>
  </vuci-form>
</template>
```
This is the basic structure of the UCI configuration page.

Once created, the `vuci-form` component loads the UCI configuration file specified by the `uci-config`
property by calling ubus. The form is then generated from the subcomponent.

## Form Validation

Form validation is implemented by specifying `rules` attribute to `vuci-form-item-xx` components. This property supports strings, objects, and custom functions.

``` vue
<vuci-form-item-input label="ID" name="id" rules="integer" :uci-section="s"/>
```
Equivalent to
``` vue
<vuci-form-item-input label="ID" name="id" :rules="{type: 'integer'}" :uci-section="s"/>
```
Equivalent to
``` vue
<vuci-form-item-input label="ID" name="id" :rules="validateID" :uci-section="s"/>
...
<script>
export default {
  methods: {
    validateID(value) {
      if (!isNaN(value) && parseInt(value).toString() === value)
        return;

      return this.$t('validator.integer');
    }
  }
}
</script>
```

Multiple rules can be specified by object:
``` vue
<vuci-form-item-input label="ID" name="id" :rules="{type: 'integer', min: 12, max: 100}" :uci-section="s"/>
```

If this option is mandatory, you can set the `required` attribute, which is of Boolean type and defaults to false.
``` vue
<vuci-form-item-input label="ID" name="id" required :uci-section="s"/>
```

Vuci currently supports validation rules:
- url
- email
- number
- integer
- uinteger
- min
- max
- hostname
- ip4addr
- ip6addr
- ipaddr
- netmask4
- netmask6
- host
- port
- macaddr
- uciname

## Option dependency

The display of options depends on the value of one or more other options.

Dependency can be implemented by setting the `depend` attribute to the `vuci-form-item-xx` component. The type of this property is a string. We need to provide an expression in string.
``` js
depend="(a == 12 || a == 'x') && y == 4 && q != 5 && !z"
```

The `a` option depends on the `b` option. If the value of the `b` option is '5', the `a` option will be displayed, otherwise it will not be displayed:
``` vue
<vuci-form-item-input label="A" name="a" depend="b == '5'" :uci-section="s"/>
<vuci-form-item-input label="B" name="b" :uci-section="s"/>
```

The `a` option depends on the `b` option and the `c` option. If the value of the `b` option is 'vuci', and the value of the `c` option
is not equal to 'u', the `a` option will be displayed, otherwise it will not be displayed:
``` vue
<vuci-form-item-input label="A" name="a" depend="b == 'vuci' && c != 'u'" :uci-section="s"/>
<vuci-form-item-input label="B" name="b" :uci-section="s"/>
<vuci-form-item-input label="C" name="c" :uci-section="s"/>
```

The `a` option depends on the `b` option and the `c` option. If the value of the `b` option is true and the value of the
`c` option is false, the a option will be displayed, otherwise it will not be displayed:
``` vue
<vuci-form-item-input label="A" name="a" depend="b && !c" :uci-section="s"/>
<vuci-form-item-switch label="B" name="b" :uci-section="s"/>
<vuci-form-item-switch label="C" name="c" :uci-section="s"/>
```
